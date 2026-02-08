#include "ArtifactsManager.hpp"
#include "../Processor/IArtifactProcessor.hpp"
#include "../AnalysisResult.hpp"

namespace apu {

    ArtifactsManager::ArtifactsManager(Config config,
                                       ResolvedBins bins,
                                       Database* database)
        : config_(std::move(config)),
          bins_(std::move(bins)),
          database_(database) {

        std::filesystem::create_directories(bins_.incoming);
        std::filesystem::create_directories(bins_.flagged);
        std::filesystem::create_directories(bins_.dismissed);
        std::filesystem::create_directories(bins_.corrupted);

        spdlog::info("ArtifactsManager initialized with bins:");
        spdlog::info("  Incoming:  {}", bins_.incoming.string());
        spdlog::info("  Flagged:   {}", bins_.flagged.string());
        spdlog::info("  Dismissed: {}", bins_.dismissed.string());
        spdlog::info("  Corrupted: {}", bins_.corrupted.string());
    }

    ArtifactsManager::~ArtifactsManager() {
        Stop();
    }

    void ArtifactsManager::Start() {
        if (running_.exchange(true)) {
            spdlog::warn("ArtifactsManager already running");
            return;
        }

        spdlog::info("Starting ArtifactsManager scan loop (interval: {}m)",
                    config_.scanInterval.count());

        scanThread_ = std::thread(&ArtifactsManager::ScanLoop, this);
    }

    void ArtifactsManager::Stop() {
        if (!running_.exchange(false)) {
            return;
        }

        spdlog::info("Stopping ArtifactsManager");

        if (scanThread_.joinable()) {
            scanThread_.join();
        }
    }

    void ArtifactsManager::RegisterProcessor(std::shared_ptr<IArtifactProcessor> processor) {
        std::lock_guard lock(processorsMutex_);
        processors_.push_back(std::move(processor));
        spdlog::info("Registered processor: {}", processors_.back()->GetName());
    }

    void ArtifactsManager::ScanLoop() {
        while (running_) {
            try {
                ProcessBatch();
            } catch (const std::exception& e) {
                spdlog::error("Error in scan loop: {}", e.what());
            }

            std::this_thread::sleep_for(config_.scanInterval);
        }
    }

    void ArtifactsManager::ProcessBatch() {
        const auto uuids = database_->GetArtifactsForProcessing(config_.processingBatchSize);

        if (uuids.empty()) {
            spdlog::debug("No artifacts to process");
            return;
        }

        spdlog::info("Processing batch of {} artifacts", uuids.size());

        for (const auto& uuid : uuids) {
            if (!running_) break;

            const auto artifactPath = bins_.incoming / uuid;

            if (!std::filesystem::exists(artifactPath)) {
                spdlog::warn("Artifact file not found: {}", uuid);
                continue;
            }

            if (!ValidateArtifact(uuid, artifactPath)) {
                const int attempts = database_->GetAttemptCount(uuid);
                if (attempts >= config_.maxProcessingAttempts) {
                    spdlog::warn("Artifact {} exceeded max attempts, marking as bad", uuid);
                    HandleBadArtifact(uuid, artifactPath);
                }
                continue;
            }

            database_->UpdateProcessingState(uuid, ArtifactState::Processing);

            bool processed = false;
            {
                std::lock_guard lock(processorsMutex_);
                for (const auto& processor : processors_) {
                    if (processor->process(uuid)) {
                        processed = true;
                        break;
                    }
                }
            }

            if (processed) {
                database_->MarkProcessed(uuid);
                MoveToDestination(uuid, artifactPath);
                spdlog::info("Successfully processed artifact: {}", uuid);
            } else {
                database_->UpdateProcessingState(uuid, ArtifactState::Received);
                spdlog::warn("No processor handled artifact: {}", uuid);
            }
        }
    }

    bool ArtifactsManager::ValidateArtifact(const std::string& uniqueId,
                                            const std::filesystem::path& artifactPath) {
        database_->IncrementAttempts(uniqueId);

        if (IsBeingReceived(artifactPath)) {
            spdlog::debug("Artifact {} still being received", uniqueId);
            return false;
        }

        const auto fileSize = std::filesystem::file_size(artifactPath);
        if (fileSize > config_.maxArtifactSizeBytes) {
            spdlog::warn("Artifact {} exceeds max size: {} bytes", uniqueId, fileSize);
            return false;
        }

        if (IsCorrupted(artifactPath)) {
            spdlog::warn("Artifact {} appears corrupted", uniqueId);
            return false;
        }

        return true;
    }

    bool ArtifactsManager::IsBeingReceived(const std::filesystem::path& path) {
        const auto initialSize = std::filesystem::file_size(path);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        const auto finalSize = std::filesystem::file_size(path);
        return initialSize != finalSize;
    }

    bool ArtifactsManager::IsCorrupted(const std::filesystem::path& path) {
        try {
            const auto typeInfo = fileTypeDetector_.Detect(path);
            return typeInfo.formatName == "UNKNOWN";
        } catch (const std::exception& e) {
            spdlog::error("Error detecting file type for {}: {}", path.string(), e.what());
            return true;
        }
    }

    void ArtifactsManager::HandleBadArtifact(const std::string& uniqueId,
                                             const std::filesystem::path& artifactPath) {
        database_->MarkBad(uniqueId);

        if (config_.badArtifactHandling == BadArtifactHandling::Delete) {
            std::filesystem::remove(artifactPath);
            spdlog::info("Deleted bad artifact: {}", uniqueId);
        } else {
            const auto destPath = bins_.corrupted / uniqueId;
            std::filesystem::rename(artifactPath, destPath);
            spdlog::info("Moved bad artifact to: {}", destPath.string());
        }
    }

    void ArtifactsManager::MoveToDestination(const std::string& uniqueId,
                                             const std::filesystem::path& sourcePath) {
        const auto metadata = database_->GetArtifactByUuid(uniqueId);
        if (!metadata) {
            spdlog::error("Failed to get metadata for {}", uniqueId);
            return;
        }

        std::filesystem::path destDir;
        switch (metadata->interestLevel) {
        case InterestLevel::Interesting:
        case InterestLevel::Critical:
            destDir = bins_.flagged;
            break;
        case InterestLevel::NotInteresting:
            destDir = bins_.dismissed;
            break;
        default:
            destDir = bins_.dismissed;
            break;
        }

        const auto destPath = destDir / uniqueId;
        std::filesystem::rename(sourcePath, destPath);
        spdlog::info("Moved artifact {} to {}", uniqueId, destDir.string());
    }

} // namespace apu
