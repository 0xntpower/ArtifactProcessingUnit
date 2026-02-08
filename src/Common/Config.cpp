#include "../pch.hpp"
#include "Config.hpp"

#include <regex>

namespace apu {

    namespace {
        constexpr char kDefaultConfig[] = R"({
    "bins": {
        "incoming": "incoming",
        "flagged": "flagged",
        "dismissed": "dismissed",
        "corrupted": "corrupted"
    },
    "scan-interval": "3m",
    "processing-batch-size": 20,
    "max-artifact-size": "10gb",
    "max-processing-attempts": 100,
    "bad-artifact-handling": "DELETE",
    "VirusTotal": {
        "use-vt": false,
        "vendors-threshold": 60
    }
})";
    }

    Config Config::Load(const std::filesystem::path& configPath) {
        Config config;

        if (!std::filesystem::exists(configPath)) {
            spdlog::warn("Config file not found at {}, using defaults", configPath.string());
            return config;
        }

        std::ifstream file(configPath);
        if (!file) {
            spdlog::error("Failed to open config file: {}", configPath.string());
            return config;
        }

        nlohmann::json json;
        try {
            file >> json;
        } catch (const std::exception& e) {
            spdlog::error("Failed to parse config JSON: {}", e.what());
            return config;
        }

        try {
            if (json.contains("scan-interval")) {
                config.scanInterval = ParseDuration(json["scan-interval"].get<std::string>());
            }

            if (json.contains("processing-batch-size")) {
                config.processingBatchSize = json["processing-batch-size"].get<int>();
            }

            if (json.contains("max-artifact-size")) {
                config.maxArtifactSizeBytes = ParseSize(json["max-artifact-size"].get<std::string>());
            }

            if (json.contains("max-processing-attempts")) {
                config.maxProcessingAttempts = json["max-processing-attempts"].get<int>();
            }

            if (json.contains("bad-artifact-handling")) {
                const std::string handling = json["bad-artifact-handling"].get<std::string>();
                if (handling == "DELETE") {
                    config.badArtifactHandling = BadArtifactHandling::Delete;
                } else if (handling == "STORE") {
                    config.badArtifactHandling = BadArtifactHandling::Store;
                }
            }

            if (json.contains("VirusTotal")) {
                const auto& vt = json["VirusTotal"];
                if (vt.contains("use-vt")) {
                    config.virusTotal.useVt = vt["use-vt"].get<bool>();
                }
                if (vt.contains("vendors-threshold")) {
                    config.virusTotal.vendorsThreshold = vt["vendors-threshold"].get<int>();
                }
            }

            if (json.contains("bins")) {
                const auto& bins = json["bins"];
                if (bins.contains("incoming")) {
                    config.bins.incoming = bins["incoming"].get<std::string>();
                }
                if (bins.contains("flagged")) {
                    config.bins.flagged = bins["flagged"].get<std::string>();
                }
                if (bins.contains("dismissed")) {
                    config.bins.dismissed = bins["dismissed"].get<std::string>();
                }
                if (bins.contains("corrupted")) {
                    config.bins.corrupted = bins["corrupted"].get<std::string>();
                }
            }

            spdlog::info("Configuration loaded from: {}", configPath.string());
        } catch (const std::exception& e) {
            spdlog::error("Error parsing config values: {}", e.what());
        }

        return config;
    }

    void Config::Save(const std::filesystem::path& configPath) const {
        nlohmann::json json;

        json["bins"]["incoming"] = bins.incoming;
        json["bins"]["flagged"] = bins.flagged;
        json["bins"]["dismissed"] = bins.dismissed;
        json["bins"]["corrupted"] = bins.corrupted;

        json["scan-interval"] = std::format("{}m", scanInterval.count());
        json["processing-batch-size"] = processingBatchSize;
        json["max-artifact-size"] = std::format("{}gb", maxArtifactSizeBytes / (1024ULL * 1024 * 1024));
        json["max-processing-attempts"] = maxProcessingAttempts;
        json["bad-artifact-handling"] = (badArtifactHandling == BadArtifactHandling::Delete) ? "DELETE" : "STORE";

        json["VirusTotal"]["use-vt"] = virusTotal.useVt;
        json["VirusTotal"]["vendors-threshold"] = virusTotal.vendorsThreshold;

        std::ofstream file(configPath);
        if (!file) {
            throw std::runtime_error("Failed to open config file for writing: " + configPath.string());
        }

        file << json.dump(4);
        spdlog::info("Configuration saved to: {}", configPath.string());
    }

    ResolvedBins Config::ResolveBins(const std::filesystem::path& baseDir) const {
        return ResolvedBins{
            baseDir / bins.incoming,
            baseDir / bins.flagged,
            baseDir / bins.dismissed,
            baseDir / bins.corrupted
        };
    }

    uint64_t Config::ParseSize(const std::string& sizeStr) {
        std::regex sizeRegex(R"((\d+(?:\.\d+)?)\s*(kb|mb|gb|tb)?)", std::regex::icase);
        std::smatch match;

        if (!std::regex_match(sizeStr, match, sizeRegex)) {
            throw std::runtime_error("Invalid size format: " + sizeStr);
        }

        const double value = std::stod(match[1].str());
        const std::string unit = match[2].str();

        uint64_t multiplier = 1;
        if (unit.empty()) {
            multiplier = 1;
        } else if (unit == "kb" || unit == "KB") {
            multiplier = 1024;
        } else if (unit == "mb" || unit == "MB") {
            multiplier = 1024ULL * 1024;
        } else if (unit == "gb" || unit == "GB") {
            multiplier = 1024ULL * 1024 * 1024;
        } else if (unit == "tb" || unit == "TB") {
            multiplier = 1024ULL * 1024 * 1024 * 1024;
        }

        return static_cast<uint64_t>(value * multiplier);
    }

    std::chrono::minutes Config::ParseDuration(const std::string& durationStr) {
        std::regex durationRegex(R"((\d+)\s*([smh])?)", std::regex::icase);
        std::smatch match;

        if (!std::regex_match(durationStr, match, durationRegex)) {
            throw std::runtime_error("Invalid duration format: " + durationStr);
        }

        const int value = std::stoi(match[1].str());
        const std::string unit = match[2].str();

        if (unit.empty() || unit == "m" || unit == "M") {
            return std::chrono::minutes(value);
        } else if (unit == "s" || unit == "S") {
            return std::chrono::minutes(value / 60);
        } else if (unit == "h" || unit == "H") {
            return std::chrono::minutes(value * 60);
        }

        return std::chrono::minutes(value);
    }

} // namespace apu
