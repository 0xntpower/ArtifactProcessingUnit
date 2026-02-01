#pragma once

#include "../IContentProcessor.hpp"

namespace apu {

	class TextProcessor final : public IContentProcessor {
	public:
		TextProcessor();
		AnalysisResult process(std::string uuid) override;
	};

} // namespace apu
