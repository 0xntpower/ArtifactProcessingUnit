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

    struct Config {
        std::chrono::minutes scanInterval{ 3 };
        int processingBatchSize = 20;
        uint64_t maxArtifactSizeBytes = 10ULL * 1024 * 1024 * 1024; // 10GB
        int maxProcessingAttempts = 100;
        BadArtifactHandling badArtifactHandling = BadArtifactHandling::Delete;
        VirusTotalConfig virusTotal;

        static Config Load(const std::filesystem::path& configPath);
        void Save(const std::filesystem::path& configPath) const;

    private:
        static uint64_t ParseSize(const std::string& sizeStr);
        static std::chrono::minutes ParseDuration(const std::string& durationStr);
    };

} // namespace apu