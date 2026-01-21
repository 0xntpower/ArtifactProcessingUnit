#pragma once

#include "../pch.hpp"

class IContentExtractor {
public:
	virtual ~IContentExtractor() = default;
	[[nodiscard]] virtual std::string extract(std::string uuid) = 0;
};
