#include "../pch.hpp"

class IContentProcessor {
public:
	virtual ~IContentProcessor() = default;
	[[nodiscard]] virtual std::string process(std::string uuid) = 0;
};
