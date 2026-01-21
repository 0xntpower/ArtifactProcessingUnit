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
                        std::filesystem::path artifactsDir,
                        std::filesystem::path interestingDir,
                        std::filesystem::path notInterestingDir,
                        std::filesystem::path badArtifactsDir,
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
        std::filesystem::path artifactsDir_;
        std::filesystem::path interestingDir_;
        std::filesystem::path notInterestingDir_;
        std::filesystem::path badArtifactsDir_;
        Database* database_;

        std::vector<std::shared_ptr<IArtifactProcessor>> processors_;
        FileTypeDetector fileTypeDetector_;

        std::atomic<bool> running_{ false };
        std::thread scanThread_;
        std::mutex processorsMutex_;
    };

} // namespace apu