#include "Database.hpp"
#include "../AnalysisResult.hpp"

namespace apu {

    namespace {
        constexpr char kCreateArtifactsTable[] = R"(
            CREATE TABLE IF NOT EXISTS artifacts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                unique_id TEXT UNIQUE NOT NULL,
                blake3_signature TEXT,
                original_name TEXT,
                state TEXT NOT NULL CHECK(state IN ('RECEIVED', 'PROCESSING', 'PROCESSED', 'BAD')),
                attempts INTEGER DEFAULT 0,
                operation TEXT,
                machine TEXT,
                ip TEXT,
                target_name TEXT,
                format TEXT,
                language TEXT,
                description TEXT,
                interest_level TEXT CHECK(interest_level IN ('NotInteresting', 'Interesting', 'Critical')),
                extracted_size_bytes INTEGER,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                processed_at DATETIME
            )
        )";

        constexpr char kCreateExtractedContentTable[] = R"(
            CREATE VIRTUAL TABLE IF NOT EXISTS extracted_content USING fts5(
                artifact_id UNINDEXED,
                content,
                tokenize='porter unicode61'
            )
        )";

        constexpr char kCreateInterestLevelIndex[] =
            "CREATE INDEX IF NOT EXISTS idx_interest_level ON artifacts(interest_level)";

        constexpr char kCreateStateIndex[] =
            "CREATE INDEX IF NOT EXISTS idx_state ON artifacts(state)";
    }

    Database::Database(const std::filesystem::path& dbPath)
        : dbPath_(dbPath) {
    }

    void Database::Initialize() {
        std::lock_guard lock(mutex_);

        db_ = std::make_unique<SQLite::Database>(dbPath_.string(),
            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        db_->exec(kCreateArtifactsTable);
        db_->exec(kCreateExtractedContentTable);
        db_->exec(kCreateInterestLevelIndex);
        db_->exec(kCreateStateIndex);

        spdlog::info("Database initialized at: {}", dbPath_.string());
    }

    int64_t Database::InsertArtifact(const std::string& uniqueId, const std::string& blake3Hash,
                                     const std::string& originalName, const std::string& operation,
                                     const std::string& machine, const std::string& ip,
                                     const std::string& targetName) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "INSERT INTO artifacts (unique_id, blake3_signature, original_name, state, "
            "operation, machine, ip, target_name) VALUES (?, ?, ?, 'RECEIVED', ?, ?, ?, ?)");

        query.bind(1, uniqueId);
        query.bind(2, blake3Hash);
        query.bind(3, originalName);
        query.bind(4, operation);
        query.bind(5, machine);
        query.bind(6, ip);
        query.bind(7, targetName);

        query.exec();
        return db_->getLastInsertRowid();
    }

    void Database::UpdateProcessingState(const std::string& uniqueId, ArtifactState state) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_, "UPDATE artifacts SET state = ? WHERE unique_id = ?");
        query.bind(1, StateToString(state));
        query.bind(2, uniqueId);
        query.exec();
    }

    void Database::IncrementAttempts(const std::string& uniqueId) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "UPDATE artifacts SET attempts = attempts + 1 WHERE unique_id = ?");
        query.bind(1, uniqueId);
        query.exec();
    }

    void Database::UpdateAnalysis(const std::string& uniqueId, const std::string& format,
                                  const std::string& language, const std::string& description,
                                  const std::string& interestLevel) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "UPDATE artifacts SET format = ?, language = ?, description = ?, "
            "interest_level = ? WHERE unique_id = ?");
        query.bind(1, format);
        query.bind(2, language);
        query.bind(3, description);
        query.bind(4, interestLevel);
        query.bind(5, uniqueId);
        query.exec();
    }

    void Database::UpdateExtractedContent(const std::string& uniqueId,
                                          const std::string& content,
                                          int64_t sizeBytes) {
        std::lock_guard lock(mutex_);

        SQLite::Statement updateSize(*db_,
            "UPDATE artifacts SET extracted_size_bytes = ? WHERE unique_id = ?");
        updateSize.bind(1, sizeBytes);
        updateSize.bind(2, uniqueId);
        updateSize.exec();

        SQLite::Statement insertContent(*db_,
            "INSERT INTO extracted_content (artifact_id, content) VALUES (?, ?)");
        insertContent.bind(1, uniqueId);
        insertContent.bind(2, content);
        insertContent.exec();
    }

    void Database::MarkProcessed(const std::string& uniqueId) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "UPDATE artifacts SET state = 'PROCESSED', processed_at = CURRENT_TIMESTAMP "
            "WHERE unique_id = ?");
        query.bind(1, uniqueId);
        query.exec();
    }

    void Database::MarkBad(const std::string& uniqueId) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "UPDATE artifacts SET state = 'BAD' WHERE unique_id = ?");
        query.bind(1, uniqueId);
        query.exec();
    }

    std::optional<ArtifactMetadata> Database::GetArtifactByUuid(const std::string& uniqueId) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "SELECT id, unique_id, blake3_signature, original_name, state, attempts, "
            "operation, machine, ip, target_name, format, language, description, "
            "interest_level, extracted_size_bytes, created_at, processed_at "
            "FROM artifacts WHERE unique_id = ?");
        query.bind(1, uniqueId);

        if (!query.executeStep()) {
            return std::nullopt;
        }

        ArtifactMetadata metadata;
        metadata.id = query.getColumn(0).getInt64();
        metadata.uniqueId = query.getColumn(1).getString();
        metadata.blake3Signature = query.getColumn(2).getString();
        metadata.originalName = query.getColumn(3).getString();
        metadata.state = StringToState(query.getColumn(4).getString());
        metadata.attempts = query.getColumn(5).getInt();
        metadata.operation = query.getColumn(6).getString();
        metadata.machine = query.getColumn(7).getString();
        metadata.ip = query.getColumn(8).getString();
        metadata.targetName = query.getColumn(9).getString();
        metadata.format = query.getColumn(10).getString();
        metadata.language = query.getColumn(11).getString();
        metadata.description = query.getColumn(12).getString();

        const std::string interestStr = query.getColumn(13).getString();
        if (interestStr == "NotInteresting") {
            metadata.interestLevel = InterestLevel::NotInteresting;
        } else if (interestStr == "Interesting") {
            metadata.interestLevel = InterestLevel::Interesting;
        } else if (interestStr == "Critical") {
            metadata.interestLevel = InterestLevel::Critical;
        } else {
            metadata.interestLevel = InterestLevel::None;
        }

        metadata.extractedSizeBytes = query.getColumn(14).getInt64();
        metadata.createdAt = query.getColumn(15).getString();
        metadata.processedAt = query.getColumn(16).getString();

        return metadata;
    }

    std::vector<std::string> Database::GetArtifactsForProcessing(int limit) {
        std::lock_guard lock(mutex_);

        std::vector<std::string> uuids;

        SQLite::Statement query(*db_,
            "SELECT unique_id FROM artifacts WHERE state = 'RECEIVED' "
            "ORDER BY created_at ASC LIMIT ?");
        query.bind(1, limit);

        while (query.executeStep()) {
            uuids.push_back(query.getColumn(0).getString());
        }

        return uuids;
    }

    int Database::GetAttemptCount(const std::string& uniqueId) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "SELECT attempts FROM artifacts WHERE unique_id = ?");
        query.bind(1, uniqueId);

        if (query.executeStep()) {
            return query.getColumn(0).getInt();
        }

        return 0;
    }

    bool Database::ArtifactExists(const std::string& blake3Hash) {
        std::lock_guard lock(mutex_);

        SQLite::Statement query(*db_,
            "SELECT COUNT(*) FROM artifacts WHERE blake3_signature = ?");
        query.bind(1, blake3Hash);

        if (query.executeStep()) {
            return query.getColumn(0).getInt() > 0;
        }

        return false;
    }

    std::string Database::StateToString(ArtifactState state) const {
        switch (state) {
        case ArtifactState::Received:   return "RECEIVED";
        case ArtifactState::Processing: return "PROCESSING";
        case ArtifactState::Processed:  return "PROCESSED";
        case ArtifactState::Bad:        return "BAD";
        }
        return "RECEIVED";
    }

    ArtifactState Database::StringToState(const std::string& state) const {
        if (state == "RECEIVED")   return ArtifactState::Received;
        if (state == "PROCESSING") return ArtifactState::Processing;
        if (state == "PROCESSED")  return ArtifactState::Processed;
        if (state == "BAD")        return ArtifactState::Bad;
        return ArtifactState::Received;
    }

} // namespace apu
