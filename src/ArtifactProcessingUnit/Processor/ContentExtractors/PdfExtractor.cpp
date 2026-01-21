#include "PdfExtractor.hpp"

PdfExtractor::PdfExtractor()
	: IContentExtractor("DocxExtractor", "Extracts all content from a pdf artifact.") {
}

std::string PdfExtractor::extract(std::string uuid) {
	spdlog::info(std::format("Extracting %s as pdf artifact.", uuid));
	return "no content found";
}
