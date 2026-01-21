#include "TextProcessor.hpp"

namespace apu {

	TextProcessor::TextProcessor()
		: IContentProcessor("TextProcessor", "Extracts all content from a docx artifact.") {
	}

	AnalysisResult TextProcessor::process(std::string uuid) {
		spdlog::info(std::format("Processing %s 's text data.", uuid));
		return {
			.description = "No description generated",
			.interestLevel = InterestLevel::None
		};
	}

}
