#include "LanguageProcessor.hpp"

namespace apu {

	LanguageProcessor::LanguageProcessor()
		: IContentProcessor("LanguageProcessor", "Detects what language the content of this document is written in.") {
	}

	AnalysisResult LanguageProcessor::process(std::string uuid) {
		spdlog::info(std::format("Processing %s 's text data.", uuid));
		return {
		   .description = "No language detected"
		};
	}

}
