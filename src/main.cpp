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

    std::string configPath = "Config.json";
    std::string dbPath = "artifacts.db";

    app.add_option("--config", configPath, "Path to config file");
    app.add_option("--database", dbPath, "Path to SQLite database file");

    CLI11_PARSE(app, argc, argv);

    // Resolve base directory from executable location
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    const std::filesystem::path baseDir = std::filesystem::path(exePath).parent_path();

    // Resolve config and database paths relative to base dir if not absolute
    if (!std::filesystem::path(configPath).is_absolute()) {
        configPath = (baseDir / configPath).string();
    }
    if (!std::filesystem::path(dbPath).is_absolute()) {
        dbPath = (baseDir / dbPath).string();
    }

    try {
        spdlog::info("=== Artifact Processing Unit Starting ===");
        spdlog::info("Base directory: {}", baseDir.string());
        spdlog::info("Config: {}", configPath);
        spdlog::info("Database: {}", dbPath);

        // Load configuration
        auto config = apu::Config::Load(configPath);

        // Resolve bin directories from config
        const auto bins = config.ResolveBins(baseDir);

        spdlog::info("Bins:");
        spdlog::info("  Incoming:  {}", bins.incoming.string());
        spdlog::info("  Flagged:   {}", bins.flagged.string());
        spdlog::info("  Dismissed: {}", bins.dismissed.string());
        spdlog::info("  Corrupted: {}", bins.corrupted.string());
        spdlog::info("Scan interval: {}m", config.scanInterval.count());
        spdlog::info("Batch size: {}", config.processingBatchSize);
        spdlog::info("Max artifact size: {} bytes", config.maxArtifactSizeBytes);
        spdlog::info("Max processing attempts: {}", config.maxProcessingAttempts);

        // Initialize database
        apu::Database database(dbPath);
        database.Initialize();

        // Create artifacts manager
        apu::ArtifactsManager manager(config, bins, &database);

        // Register artifact processors
        manager.RegisterProcessor(
            std::make_shared<apu::DocxProcessor>(&database, bins.incoming)
        );
        manager.RegisterProcessor(
            std::make_shared<apu::PdfProcessor>(&database, bins.incoming)
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