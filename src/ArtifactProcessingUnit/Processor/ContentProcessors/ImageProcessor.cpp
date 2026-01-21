#include "ImageProcessor.hpp"

namespace apu {

	ImageProcessor::ImageProcessor()
		: IContentProcessor("ImageProcessor", "Extracts all content from a docx artifact.") {
	}

	AnalysisResult ImageProcessor::process(std::string uuid) {
		spdlog::info(std::format("Processing %s 's image data.", uuid));
		return {
            .language = "No image description generated"
		};
	}

}

