#pragma once

#include "../pch.hpp"
#include "../Common/Config.hpp"
#include "../Database/Database.hpp"
#include "../Common/FileTypeDetector.hpp"

namespace apu {

    class IArtifactProcessor;

    class ArtifactsManager {
    public:
        ArtifactsManager(Config config,
                        ResolvedBins bins,
                        Database* database);

        ~ArtifactsManager();

        void Start();
        void Stop();

        void RegisterProcessor(std::shared_ptr<IArtifactProcessor> processor);

    private:
        void ScanLoop();
        void ProcessBatch();
        bool ValidateArtifact(const std::string& uniqueId, const std::filesystem::path& artifactPath);
        bool IsBeingReceived(const std::filesystem::path& path);
        bool IsCorrupted(const std::filesystem::path& path);
        void HandleBadArtifact(const std::string& uniqueId, const std::filesystem::path& artifactPath);
        void MoveToDestination(const std::string& uniqueId, const std::filesystem::path& sourcePath);

        Config config_;
        ResolvedBins bins_;
        Database* database_;

        std::vector<std::shared_ptr<IArtifactProcessor>> processors_;
        FileTypeDetector fileTypeDetector_;

        std::atomic<bool> running_{ false };
        std::thread scanThread_;
        std::mutex processorsMutex_;
    };

} // namespace apu