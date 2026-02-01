#include "../IContentExtractor.hpp"

namespace apu {

	class PdfExtractor final : public IContentExtractor {
	public:
		std::string extract(std::string uuid) override;
	};

} // namespace apu
