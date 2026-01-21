#pragma once

#include "../pch.hpp"

namespace apu {

	class IContentExtractor {
	public:
		virtual ~IContentExtractor() = default;
		[[nodiscard]] virtual std::string extract(std::string uuid) = 0;
	};

} // namespace apu
