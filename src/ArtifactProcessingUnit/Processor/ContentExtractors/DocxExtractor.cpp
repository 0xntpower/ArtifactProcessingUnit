#include "DocxExtractor.hpp"

namespace apu {

	std::string DocxExtractor::extract(std::string uuid) {
		spdlog::info(std::format("Extracting %s as docx artifact.", uuid));
		return "no content found";
	}

}
