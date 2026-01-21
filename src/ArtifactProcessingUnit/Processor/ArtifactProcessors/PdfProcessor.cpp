#include "PdfProcessor.hpp"

#include "../../AnalysisResult.hpp"

#include "../ContentExtractors/PdfExtractor.hpp"

#include "../ContentProcessors/LanguageProcessor.hpp"
#include "../ContentProcessors/TextProcessor.hpp"
#include "../ContentProcessors/ImageProcessor.hpp"

namespace apu {

	PdfProcessor::PdfProcessor()
		: IArtifactProcessor("PdfProcessor", "Extracts all content and figures our what it is from pdf documents.") {
	}

	AnalysisResult PdfProcessor::process(std::string uuid) {
		spdlog::info(std::format("Processing %s as pdf artifact.", uuid));
		return true;
	}

}
