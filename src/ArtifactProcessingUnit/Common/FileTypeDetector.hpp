#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <array>
#include <optional>

namespace apu {

    struct FileTypeInfo {
        std::string mimeType;
        std::string formatName;  // Short name for Analysis.Format field
    };

    class FileTypeDetector {
    public:
        [[nodiscard]] FileTypeInfo Detect(const std::filesystem::path& path) const;
        [[nodiscard]] std::string DetectMime(const std::filesystem::path& path) const;
        [[nodiscard]] std::string DetectFormat(const std::filesystem::path& path) const;

    private:
        static constexpr std::size_t kHeaderSize = 64;

        [[nodiscard]] std::optional<FileTypeInfo> DetectFromHeader(
            const std::array<unsigned char, kHeaderSize>& header,
            std::size_t bytesRead
        ) const;

        [[nodiscard]] FileTypeInfo DetectZipContents(const std::filesystem::path& path) const;
        [[nodiscard]] FileTypeInfo DetectOleContents(const std::filesystem::path& path) const;
    };

} // namespace apu