#include "pch.hpp"
#include <CLI/CLI.hpp>

#include "Common/Config.hpp"
#include "Database/Database.hpp"
#include "Manager/ArtifactsManager.hpp"
#include "Processor/ArtifactProcessors/DocxProcessor.hpp"
#include "Processor/ArtifactProcessors/PdfProcessor.hpp"

#include <csignal>

namespace {
    std::atomic<bool> g_running{ true };

    void SignalHandler(int signal) {
        if (signal == SIGINT || signal == SIGTERM) {
            spdlog::info("Received shutdown signal");
            g_running = false;
        }
    }
}

int main(int argc, char* argv[]) {
    CLI::App app{ "ArtifactProcessingUnit - Document Processing and Classification System" };

    std::string artifactsDir;
    std::string interestingDir;
    std::string notInterestingDir;
    std::string badArtifactsDir;
    std::string configPath = "Config.json";
    std::string dbPath = "artifacts.db";
    bool detectLocal = false;

    app.add_option("--artifacts", artifactsDir, "Path to artifacts directory");
    app.add_option("--interesting", interestingDir, "Path to interesting artifacts directory");
    app.add_option("--notinteresting", notInterestingDir, "Path to not interesting artifacts directory");
    app.add_option("--badartifacts", badArtifactsDir, "Path to bad artifacts directory");
    app.add_option("--config", configPath, "Path to config file");
    app.add_option("--database", dbPath, "Path to SQLite database file");
    app.add_flag("--detectlocal", detectLocal, "Auto-detect folders in executable directory");

    CLI11_PARSE(app, argc, argv);

    if (detectLocal) {
        char exePath[MAX_PATH];
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

        artifactsDir = (exeDir / "artifacts").string();
        interestingDir = (exeDir / "interesting").string();
        notInterestingDir = (exeDir / "notinteresting").string();
        badArtifactsDir = (exeDir / "badartifacts").string();
        configPath = (exeDir / "Config.json").string();
        dbPath = (exeDir / "artifacts.db").string();

        spdlog::info("Using local detection from: {}", exeDir.string());
    }

    if (artifactsDir.empty() || interestingDir.empty() ||
        notInterestingDir.empty() || badArtifactsDir.empty()) {
        spdlog::error("All directory paths must be specified. Use --detectlocal or provide paths manually.");
        return 1;
    }

    try {
        spdlog::info("=== Artifact Processing Unit Starting ===");
        spdlog::info("Artifacts: {}", artifactsDir);
        spdlog::info("Interesting: {}", interestingDir);
        spdlog::info("Not Interesting: {}", notInterestingDir);
        spdlog::info("Bad Artifacts: {}", badArtifactsDir);
        spdlog::info("Config: {}", configPath);
        spdlog::info("Database: {}", dbPath);

        // Load configuration
        auto config = apu::Config::Load(configPath);
        spdlog::info("Scan interval: {}m", config.scanInterval.count());
        spdlog::info("Batch size: {}", config.processingBatchSize);
        spdlog::info("Max artifact size: {} bytes", config.maxArtifactSizeBytes);
        spdlog::info("Max processing attempts: {}", config.maxProcessingAttempts);

        // Initialize database
        apu::Database database(dbPath);
        database.Initialize();

        // Create artifacts manager
        apu::ArtifactsManager manager(
            config,
            artifactsDir,
            interestingDir,
            notInterestingDir,
            badArtifactsDir,
            &database
        );

        // Register artifact processors
        manager.RegisterProcessor(
            std::make_shared<apu::DocxProcessor>(&database, artifactsDir)
        );
        manager.RegisterProcessor(
            std::make_shared<apu::PdfProcessor>(&database, artifactsDir)
        );

        // Setup signal handlers
        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);

        // Start processing
        manager.Start();

        spdlog::info("System running. Press Ctrl+C to stop.");

        // Main loop
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Graceful shutdown
        spdlog::info("Shutting down...");
        manager.Stop();

        spdlog::info("=== Artifact Processing Unit Stopped ===");
        return 0;

    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}