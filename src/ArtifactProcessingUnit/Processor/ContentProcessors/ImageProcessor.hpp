#include "../IContentProcessor.hpp"

class ImageProcessor final : public IContentProcessor {
public:
	ImageProcessor();
	bool process(std::string uuid) override;
};
