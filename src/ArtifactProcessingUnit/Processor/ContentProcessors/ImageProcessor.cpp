#include "ImageProcessor.hpp"

namespace apu {

    ImageProcessor::ImageProcessor()
        : IContentProcessor("ImageProcessor", "Processes image content using vision AI") {
    }

    AnalysisResult ImageProcessor::process(std::string uuid) {
        spdlog::info("Processing {} image data", uuid);

        // TODO: Implement image processing
        // - OCR for scanned documents
        // - Vision LLM for image analysis
        // - Metadata extraction

        return {
            .description = "Image processing not yet implemented",
            .interestLevel = InterestLevel::None,
            .language = ""
        };
    }

}
