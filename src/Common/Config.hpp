#pragma once

#include "../pch.hpp"

namespace apu {

    enum class BadArtifactHandling {
        Delete,
        Store
    };

    struct VirusTotalConfig {
        bool useVt = false;
        int vendorsThreshold = 60;
    };

    struct BinsConfig {
        std::string incoming = "incoming";
        std::string flagged = "flagged";
        std::string dismissed = "dismissed";
        std::string corrupted = "corrupted";
    };

    struct ResolvedBins {
        std::filesystem::path incoming;
        std::filesystem::path flagged;
        std::filesystem::path dismissed;
        std::filesystem::path corrupted;
    };

    struct Config {
        BinsConfig bins;
        std::chrono::minutes scanInterval{ 3 };
        int processingBatchSize = 20;
        uint64_t maxArtifactSizeBytes = 10ULL * 1024 * 1024 * 1024; // 10GB
        int maxProcessingAttempts = 100;
        BadArtifactHandling badArtifactHandling = BadArtifactHandling::Delete;
        VirusTotalConfig virusTotal;

        static Config Load(const std::filesystem::path& configPath);
        void Save(const std::filesystem::path& configPath) const;

        ResolvedBins ResolveBins(const std::filesystem::path& baseDir) const;

    private:
        static uint64_t ParseSize(const std::string& sizeStr);
        static std::chrono::minutes ParseDuration(const std::string& durationStr);
    };

} // namespace apu