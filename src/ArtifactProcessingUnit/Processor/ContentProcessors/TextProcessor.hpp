#include "../IContentProcessor.hpp"

class TextProcessor final : public IContentProcessor {
public:
	TextProcessor();
	std::string process(std::string uuid) override;
};
