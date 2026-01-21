#include "../IContentExtractor.hpp"

class DocxExtractor final : public IContentExtractor {
public:
	DocxExtractor();
	std::string extract(std::string uuid) override;
};
