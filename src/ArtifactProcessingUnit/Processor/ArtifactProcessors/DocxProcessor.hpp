#pragma once

#include "../IArtifactProcessor.hpp"
#include "../../AnalysisResult.hpp"

namespace apu {

	class DocxProcessor final : public IArtifactProcessor {
	public:
		DocxProcessor();
		bool process(std::string uuid) override;
	};

} // namespace apu