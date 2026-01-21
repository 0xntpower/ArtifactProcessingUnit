#pragma once

#include "../pch.hpp"

class IArtifactProcessor {
public:
	virtual ~IArtifactProcessor() = default;
	[[nodiscard]] virtual bool process(std::string uuid) = 0;
	std::string GetName() const noexcept { return name_; }
protected:
	IArtifactProcessor(std::string name, std::string desc)
		: name_(std::move(name)), desc_(std::move(desc)) {}
private:
	std::string name_;
	std::string desc_;
};
