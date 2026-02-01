#pragma once

#include "../IContentProcessor.hpp"

namespace apu {

	class ImageProcessor final : public IContentProcessor {
	public:
		ImageProcessor();
		AnalysisResult process(std::string uuid) override;
	};

} // namespace apu
