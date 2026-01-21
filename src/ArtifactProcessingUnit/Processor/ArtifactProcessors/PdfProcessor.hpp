#include "../IArtifactProcessor.hpp"

namespace apu {

	class PdfProcessor final : public IArtifactProcessor {
	public:
		PdfProcessor();
		bool process(std::string uuid) override;
	};

} // namespace apu