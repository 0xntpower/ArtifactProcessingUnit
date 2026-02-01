#include "DocxExtractor.hpp"

namespace apu {

    namespace {
        std::string ExtractTextFromXml(const std::string& xmlContent) {
            pugi::xml_document doc;
            const auto result = doc.load_string(xmlContent.c_str());

            if (!result) {
                spdlog::error("Failed to parse DOCX XML: {}", result.description());
                return "";
            }

            std::string text;

            // Navigate to document body and extract text from all <w:t> elements
            const auto body = doc.select_node("//w:body").node();
            if (!body) {
                return "";
            }

            for (const auto& textNode : body.select_nodes(".//w:t")) {
                const auto nodeText = textNode.node().text().get();
                if (nodeText && nodeText[0] != '\0') {
                    text += nodeText;
                    text += ' ';
                }
            }

            return text;
        }
    }

    std::string DocxExtractor::extract(std::string uuid) {
        spdlog::info("Extracting {} as DOCX artifact", uuid);

        int err = 0;
        zip_t* archive = zip_open(uuid.c_str(), ZIP_RDONLY, &err);

        if (!archive) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            spdlog::error("Failed to open DOCX archive {}: {}", uuid, zip_error_strerror(&error));
            zip_error_fini(&error);
            return "";
        }

        struct ZipGuard {
            zip_t* archive;
            ~ZipGuard() { if (archive) zip_close(archive); }
        } guard{ archive };

        // Open the main document XML
        zip_file_t* docFile = zip_fopen(archive, "word/document.xml", 0);
        if (!docFile) {
            spdlog::error("Failed to find word/document.xml in {}", uuid);
            return "";
        }

        struct ZipFileGuard {
            zip_file_t* file;
            ~ZipFileGuard() { if (file) zip_fclose(file); }
        } fileGuard{ docFile };

        // Read the entire XML content
        zip_stat_t stat;
        if (zip_stat(archive, "word/document.xml", 0, &stat) != 0) {
            spdlog::error("Failed to stat word/document.xml in {}", uuid);
            return "";
        }

        std::string xmlContent(stat.size, '\0');
        const auto bytesRead = zip_fread(docFile, xmlContent.data(), stat.size);

        if (bytesRead != static_cast<zip_int64_t>(stat.size)) {
            spdlog::error("Failed to read complete document.xml from {}", uuid);
            return "";
        }

        const auto extractedText = ExtractTextFromXml(xmlContent);

        if (extractedText.empty()) {
            spdlog::warn("No text content extracted from {}", uuid);
            return "";
        }

        spdlog::info("Extracted {} bytes of text from {}", extractedText.size(), uuid);
        return extractedText;
    }

}
