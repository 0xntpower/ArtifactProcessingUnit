#include "DocxProcessor.hpp"

#include "../../AnalysisResult.hpp"

#include "../ContentExtractors/DocxExtractor.hpp"

#include "../ContentProcessors/LanguageProcessor.hpp"
#include "../ContentProcessors/TextProcessor.hpp"

namespace apu {

	DocxProcessor::DocxProcessor()
		: IArtifactProcessor("DocxProcessor", "Extracts all content and figures our what it is from docx documents.") {
	}

	bool DocxProcessor::process(std::string uuid) {
		spdlog::info(std::format("Processing %s as docx artifact.", uuid));

		InterestLevel interestLevel = InterestLevel::None;

		std::unique_ptr<DocxExtractor> docxExtractor = std::make_unique<DocxExtractor>();
		std::string content = docxExtractor->extract(uuid);
		// TODO write content to MetaData in db

		AnalysisResult analysisResult;

		std::unique_ptr<LanguageProcessor> langExtractor = std::make_unique<LanguageProcessor>();
		analysisResult = langExtractor->process(uuid);
		// TODO write language to MetaData in db

		std::unique_ptr<TextProcessor> textExtractor = std::make_unique<TextProcessor>();
		analysisResult = textExtractor->process(uuid);
		interestLevel = analysisResult.interestLevel;
		// TODO write interestLevel and description to MetaData in db

		if (interestLevel == InterestLevel::Critical) {
			spdlog::info("Sending email alert about critical information.");
			// TODO send an email to someone
		}

	    return true;
	}

}
