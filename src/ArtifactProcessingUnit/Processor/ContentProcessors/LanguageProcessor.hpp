#pragma once

#include "../IContentProcessor.hpp"

namespace apu {

	class LanguageProcessor final : public IContentProcessor {
	public:
		LanguageProcessor();
		AnalysisResult process(std::string uuid) override;
	};

} // namespace apu
