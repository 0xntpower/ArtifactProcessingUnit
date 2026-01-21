#include "pch.hpp"
#include <CLI/CLI.hpp>

int main(int argc, char* argv[]) {
    CLI::App app{ "ArtifactProcessingUnit" };

    std::string artifactsDir;
    std::string interestingDir;
    std::string notInterestingDir;
    std::string badArtifactsDir;
    std::string configPath = "Config.json";
    bool detectLocal = false;

    app.add_option("--artifacts", artifactsDir, "Path to artifacts directory");
    app.add_option("--interesting", interestingDir, "Path to interesting artifacts directory");
    app.add_option("--notinteresting", notInterestingDir, "Path to not interesting artifacts directory");
    app.add_option("--badartifacts", badArtifactsDir, "Path to bad artifacts directory");
    app.add_option("--config", configPath, "Path to config file");
    app.add_flag("--detectlocal", detectLocal, "Auto-detect folders in executable directory");

    CLI11_PARSE(app, argc, argv);

    if (detectLocal) {
        // Get executable directory
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

        artifactsDir = (exeDir / "artifacts").string();
        interestingDir = (exeDir / "interesting").string();
        notInterestingDir = (exeDir / "notinteresting").string();
        badArtifactsDir = (exeDir / "badartifacts").string();
        configPath = (exeDir / "Config.json").string();

        spdlog::info("Using local detection from: {}", exeDir.string());
    }

    // Validate paths are set
    if (artifactsDir.empty() || interestingDir.empty() ||
        notInterestingDir.empty() || badArtifactsDir.empty()) {
        spdlog::error("All directory paths must be specified. Use --detectlocal or provide paths manually.");
        return 1;
    }

    // Validate directories exist
    if (!std::filesystem::exists(artifactsDir)) {
        spdlog::error("Artifacts directory does not exist: {}", artifactsDir);
        return 1;
    }

    spdlog::info("Directories configured:\n");
    spdlog::info("Artifacts: {}", artifactsDir);
    spdlog::info("Interesting: {}", interestingDir);
    spdlog::info("Not Interesting: {}", notInterestingDir);
    spdlog::info("Bad Artifacts: {}", badArtifactsDir);
    spdlog::info("Config: {}", configPath);

    return 0;
}