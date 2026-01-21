#include "DocxExtractor.hpp"

DocxExtractor::DocxExtractor()
	: IContentExtractor("DocxExtractor", "Extracts all content from a docx artifact.") {
}

std::string DocxExtractor::extract(std::string uuid) {
	spdlog::info(std::format("Extracting %s as docx artifact.", uuid));
	return "no content found";
}
