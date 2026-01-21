#include "PdfProcessor.hpp"
#include "../ContentExtractors/PdfExtractor.hpp"
#include "../ContentProcessors/LanguageProcessor.hpp"
#include "../ContentProcessors/TextProcessor.hpp"

namespace apu {

    PdfProcessor::PdfProcessor(Database* database, const std::filesystem::path& artifactsDir)
        : IArtifactProcessor("PdfProcessor", "Extracts content and analyzes PDF documents"),
          database_(database),
          artifactsDir_(artifactsDir) {
    }

    bool PdfProcessor::process(std::string uuid) {
        const auto artifactPath = artifactsDir_ / uuid;

        if (!std::filesystem::exists(artifactPath)) {
            spdlog::error("Artifact file not found: {}", uuid);
            return false;
        }

        // Check if this is actually a PDF file
        const auto fileType = fileTypeDetector_.DetectFormat(artifactPath);
        if (fileType != "PDF") {
            spdlog::debug("Skipping non-PDF file: {} (type: {})", uuid, fileType);
            return false;
        }

        spdlog::info("Processing {} as PDF artifact", uuid);

        // Extract content
        PdfExtractor extractor;
        const std::string content = extractor.extract(artifactPath.string());

        if (content.empty()) {
            spdlog::warn("No content extracted from {}", uuid);
            return false;
        }

        // Store extracted content in database
        database_->UpdateExtractedContent(uuid, content, static_cast<int64_t>(content.size()));

        // Detect language
        LanguageProcessor langProcessor;
        auto langResult = langProcessor.process(uuid);

        // Analyze content with LLM
        TextProcessor textProcessor;
        auto textResult = textProcessor.process(uuid);

        // Update analysis in database
        database_->UpdateAnalysis(uuid,
                                 fileType,
                                 langResult.language,
                                 textResult.description,
                                 ToString(textResult.interestLevel));

        // Alert on critical findings
        if (textResult.interestLevel == InterestLevel::Critical) {
            spdlog::warn("CRITICAL artifact detected: {} - {}", uuid, textResult.description);
            // TODO: Implement email alerting system
        }

        return true;
    }

} // namespace apu
