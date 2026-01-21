#pragma once

#include "../pch.hpp"
#include "../AnalysisResult.hpp"

namespace apu {

	class IContentProcessor {
	public:
		virtual ~IContentProcessor() = default;
		[[nodiscard]] virtual AnalysisResult process(std::string uuid) = 0;
	protected:
		IContentProcessor(std::string name, std::string desc)
			: name_(std::move(name)), desc_(std::move(desc)) {}
	private:
		std::string name_;
		std::string desc_;
	};

} // namespace apu
