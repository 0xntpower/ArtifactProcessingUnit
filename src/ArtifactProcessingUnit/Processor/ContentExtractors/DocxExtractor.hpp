#include "../IContentExtractor.hpp"

namespace apu {

	class DocxExtractor final : public IContentExtractor {
	public:
		std::string extract(std::string uuid) override;
	};

} // namespace apu
