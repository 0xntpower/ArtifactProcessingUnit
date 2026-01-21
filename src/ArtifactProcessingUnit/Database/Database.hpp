#pragma once

#include "../pch.hpp"

namespace apu {

    enum class ArtifactState {
        Received,
        Processing,
        Processed,
        Bad
    };

    enum class InterestLevel;

    struct ArtifactMetadata {
        int64_t id;
        std::string uniqueId;
        std::string blake3Signature;
        std::string originalName;
        ArtifactState state;
        int attempts;
        std::string operation;
        std::string machine;
        std::string ip;
        std::string targetName;
        std::string format;
        std::string language;
        std::string description;
        InterestLevel interestLevel;
        int64_t extractedSizeBytes;
        std::string createdAt;
        std::string processedAt;
    };

    class Database {
    public:
        explicit Database(const std::filesystem::path& dbPath);
        ~Database() = default;

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;

        void Initialize();

        int64_t InsertArtifact(const std::string& uniqueId, const std::string& blake3Hash,
                               const std::string& originalName, const std::string& operation,
                               const std::string& machine, const std::string& ip,
                               const std::string& targetName);

        void UpdateProcessingState(const std::string& uniqueId, ArtifactState state);
        void IncrementAttempts(const std::string& uniqueId);
        void UpdateAnalysis(const std::string& uniqueId, const std::string& format,
                           const std::string& language, const std::string& description,
                           const std::string& interestLevel);
        void UpdateExtractedContent(const std::string& uniqueId, const std::string& content,
                                    int64_t sizeBytes);
        void MarkProcessed(const std::string& uniqueId);
        void MarkBad(const std::string& uniqueId);

        std::optional<ArtifactMetadata> GetArtifactByUuid(const std::string& uniqueId);
        std::vector<std::string> GetArtifactsForProcessing(int limit);
        int GetAttemptCount(const std::string& uniqueId);
        bool ArtifactExists(const std::string& blake3Hash);

    private:
        std::filesystem::path dbPath_;
        std::unique_ptr<SQLite::Database> db_;
        std::mutex mutex_;

        [[nodiscard]] std::string StateToString(ArtifactState state) const;
        [[nodiscard]] ArtifactState StringToState(const std::string& state) const;
    };

} // namespace apu