#pragma once

#include "../IArtifactProcessor.hpp"
#include "../../AnalysisResult.hpp"
#include "../../Database/Database.hpp"
#include "../../Common/FileTypeDetector.hpp"

namespace apu {

    class DocxProcessor final : public IArtifactProcessor {
    public:
        DocxProcessor(Database* database, const std::filesystem::path& artifactsDir);
        bool process(std::string uuid) override;

    private:
        Database* database_;
        std::filesystem::path artifactsDir_;
        FileTypeDetector fileTypeDetector_;
    };

} // namespace apu