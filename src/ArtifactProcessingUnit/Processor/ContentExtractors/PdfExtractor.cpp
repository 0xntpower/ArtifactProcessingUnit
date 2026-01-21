#include "PdfExtractor.hpp"

namespace apu {

	std::string PdfExtractor::extract(std::string uuid) {
		spdlog::info(std::format("Extracting %s as pdf artifact.", uuid));
		return "no content found";
	}

}
