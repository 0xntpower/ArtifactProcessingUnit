#include "../pch.hpp"
#include "FileTypeDetector.hpp"

#include <fstream>
#include <algorithm>

#include <zip.h>

/*
* This is a temporary solution because these is a weird problem
* with trying to use libmagic on Windows.
* TODO implement something better or use a library.
*/

namespace apu {

    namespace {
        // Magic byte signatures
        constexpr std::array<unsigned char, 4> kMagicPdf = { 0x25, 0x50, 0x44, 0x46 };
        constexpr std::array<unsigned char, 4> kMagicZip = { 0x50, 0x4B, 0x03, 0x04 };
        constexpr std::array<unsigned char, 4> kMagicZipEmpty = { 0x50, 0x4B, 0x05, 0x06 };
        constexpr std::array<unsigned char, 4> kMagicZipSpanned = { 0x50, 0x4B, 0x07, 0x08 };
        constexpr std::array<unsigned char, 8> kMagicPng = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
        constexpr std::array<unsigned char, 3> kMagicJpeg = { 0xFF, 0xD8, 0xFF };
        constexpr std::array<unsigned char, 6> kMagicGif87a = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61 };
        constexpr std::array<unsigned char, 6> kMagicGif89a = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61 };
        constexpr std::array<unsigned char, 4> kMagicWebp = { 0x57, 0x45, 0x42, 0x50 };
        constexpr std::array<unsigned char, 4> kMagicRiff = { 0x52, 0x49, 0x46, 0x46 };
        constexpr std::array<unsigned char, 8> kMagicOle = { 0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1 };
        constexpr std::array<unsigned char, 4> kMagicFtyp = { 0x66, 0x74, 0x79, 0x70 };
        constexpr std::array<unsigned char, 4> kMagicMatroska = { 0x1A, 0x45, 0xDF, 0xA3 };
        constexpr std::array<unsigned char, 4> kMagicFlac = { 0x66, 0x4C, 0x61, 0x43 };
        constexpr std::array<unsigned char, 3> kMagicId3 = { 0x49, 0x44, 0x33 };
        constexpr std::array<unsigned char, 2> kMagicMp3Sync = { 0xFF, 0xFB };
        constexpr std::array<unsigned char, 4> kMagicOgg = { 0x4F, 0x67, 0x67, 0x53 };
        constexpr std::array<unsigned char, 4> kMagicWav = { 0x57, 0x41, 0x56, 0x45 };
        constexpr std::array<unsigned char, 4> kMagicAvi = { 0x41, 0x56, 0x49, 0x20 };
        constexpr std::array<unsigned char, 4> kMagicTiffLe = { 0x49, 0x49, 0x2A, 0x00 };
        constexpr std::array<unsigned char, 4> kMagicTiffBe = { 0x4D, 0x4D, 0x00, 0x2A };
        constexpr std::array<unsigned char, 4> kMagicRar = { 0x52, 0x61, 0x72, 0x21 };
        constexpr std::array<unsigned char, 6> kMagic7z = { 0x37, 0x7A, 0xBC, 0xAF, 0x27, 0x1C };
        constexpr std::array<unsigned char, 3> kMagicGzip = { 0x1F, 0x8B, 0x08 };
        constexpr std::array<unsigned char, 3> kMagicBz2 = { 0x42, 0x5A, 0x68 };
        constexpr std::array<unsigned char, 6> kMagicXz = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
        constexpr std::array<unsigned char, 5> kMagicXml = { 0x3C, 0x3F, 0x78, 0x6D, 0x6C };
        constexpr std::array<unsigned char, 4> kMagicWasm = { 0x00, 0x61, 0x73, 0x6D };
        constexpr std::array<unsigned char, 4> kMagicElf = { 0x7F, 0x45, 0x4C, 0x46 };
        constexpr std::array<unsigned char, 2> kMagicMz = { 0x4D, 0x5A };

        template<std::size_t N>
        bool MatchesMagic(const std::array<unsigned char, 64>& header, std::size_t bytesRead,
            const std::array<unsigned char, N>& magic, std::size_t offset = 0) {
            if (bytesRead < offset + N) {
                return false;
            }
            return std::equal(magic.begin(), magic.end(), header.begin() + offset);
        }

        std::string ToLower(std::string_view str) {
            std::string result(str);
            std::transform(result.begin(), result.end(), result.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return result;
        }
    }

    FileTypeInfo FileTypeDetector::Detect(const std::filesystem::path& path) const {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + path.string());
        }

        std::array<unsigned char, kHeaderSize> header{};
        file.read(reinterpret_cast<char*>(header.data()), kHeaderSize);
        const auto bytesRead = static_cast<std::size_t>(file.gcount());

        if (bytesRead < 4) {
            return { "application/octet-stream", "UNKNOWN" };
        }

        if (auto result = DetectFromHeader(header, bytesRead)) {
            // ZIP-based formats need further inspection
            if (result->formatName == "ZIP") {
                return DetectZipContents(path);
            }
            // OLE-based formats (legacy Office) need further inspection
            if (result->formatName == "OLE") {
                return DetectOleContents(path);
            }
            return *result;
        }

        return { "application/octet-stream", "UNKNOWN" };
    }

    std::string FileTypeDetector::DetectMime(const std::filesystem::path& path) const {
        return Detect(path).mimeType;
    }

    std::string FileTypeDetector::DetectFormat(const std::filesystem::path& path) const {
        return Detect(path).formatName;
    }

    std::optional<FileTypeInfo> FileTypeDetector::DetectFromHeader(
        const std::array<unsigned char, kHeaderSize>& header,
        std::size_t bytesRead
    ) const {
        // PDF
        if (MatchesMagic(header, bytesRead, kMagicPdf)) {
            return FileTypeInfo{ "application/pdf", "PDF" };
        }

        // ZIP-based (Office 2007+, JAR, APK, etc.)
        if (MatchesMagic(header, bytesRead, kMagicZip) ||
            MatchesMagic(header, bytesRead, kMagicZipEmpty) ||
            MatchesMagic(header, bytesRead, kMagicZipSpanned)) {
            return FileTypeInfo{ "application/zip", "ZIP" };
        }

        // PNG
        if (MatchesMagic(header, bytesRead, kMagicPng)) {
            return FileTypeInfo{ "image/png", "PNG" };
        }

        // JPEG
        if (MatchesMagic(header, bytesRead, kMagicJpeg)) {
            return FileTypeInfo{ "image/jpeg", "JPEG" };
        }

        // GIF
        if (MatchesMagic(header, bytesRead, kMagicGif87a) ||
            MatchesMagic(header, bytesRead, kMagicGif89a)) {
            return FileTypeInfo{ "image/gif", "GIF" };
        }

        // RIFF-based (WAV, AVI, WEBP)
        if (MatchesMagic(header, bytesRead, kMagicRiff)) {
            if (MatchesMagic(header, bytesRead, kMagicWebp, 8)) {
                return FileTypeInfo{ "image/webp", "WEBP" };
            }
            if (MatchesMagic(header, bytesRead, kMagicWav, 8)) {
                return FileTypeInfo{ "audio/wav", "WAV" };
            }
            if (MatchesMagic(header, bytesRead, kMagicAvi, 8)) {
                return FileTypeInfo{ "video/x-msvideo", "AVI" };
            }
            return FileTypeInfo{ "application/octet-stream", "RIFF" };
        }

        // OLE Compound Document (legacy Office: DOC, XLS, PPT)
        if (MatchesMagic(header, bytesRead, kMagicOle)) {
            return FileTypeInfo{ "application/x-ole-storage", "OLE" };
        }

        // MP4/MOV/M4A/3GP (ISO Base Media)
        if (bytesRead >= 12 && MatchesMagic(header, bytesRead, kMagicFtyp, 4)) {
            const std::string brand(reinterpret_cast<const char*>(header.data() + 8), 4);

            if (brand == "isom" || brand == "iso2" || brand == "mp41" || brand == "mp42" ||
                brand == "avc1" || brand == "dash") {
                return FileTypeInfo{ "video/mp4", "MP4" };
            }
            if (brand == "qt  " || brand == "moov") {
                return FileTypeInfo{ "video/quicktime", "MOV" };
            }
            if (brand == "M4A " || brand == "M4B ") {
                return FileTypeInfo{ "audio/mp4", "M4A" };
            }
            if (brand == "3gp4" || brand == "3gp5" || brand == "3gp6") {
                return FileTypeInfo{ "video/3gpp", "3GP" };
            }
            return FileTypeInfo{ "video/mp4", "MP4" };
        }

        // Matroska/WebM
        if (MatchesMagic(header, bytesRead, kMagicMatroska)) {
            return FileTypeInfo{ "video/x-matroska", "MKV" };
        }

        // FLAC
        if (MatchesMagic(header, bytesRead, kMagicFlac)) {
            return FileTypeInfo{ "audio/flac", "FLAC" };
        }

        // MP3 (ID3 tag or frame sync)
        if (MatchesMagic(header, bytesRead, kMagicId3) ||
            MatchesMagic(header, bytesRead, kMagicMp3Sync)) {
            return FileTypeInfo{ "audio/mpeg", "MP3" };
        }

        // OGG
        if (MatchesMagic(header, bytesRead, kMagicOgg)) {
            return FileTypeInfo{ "audio/ogg", "OGG" };
        }

        // BMP
        if (header[0] == 0x42 && header[1] == 0x4D) {
            return FileTypeInfo{ "image/bmp", "BMP" };
        }

        // TIFF
        if (MatchesMagic(header, bytesRead, kMagicTiffLe) ||
            MatchesMagic(header, bytesRead, kMagicTiffBe)) {
            return FileTypeInfo{ "image/tiff", "TIFF" };
        }

        // Archives
        if (MatchesMagic(header, bytesRead, kMagicRar)) {
            return FileTypeInfo{ "application/vnd.rar", "RAR" };
        }
        if (MatchesMagic(header, bytesRead, kMagic7z)) {
            return FileTypeInfo{ "application/x-7z-compressed", "7Z" };
        }
        if (MatchesMagic(header, bytesRead, kMagicGzip)) {
            return FileTypeInfo{ "application/gzip", "GZIP" };
        }
        if (MatchesMagic(header, bytesRead, kMagicBz2)) {
            return FileTypeInfo{ "application/x-bzip2", "BZ2" };
        }
        if (MatchesMagic(header, bytesRead, kMagicXz)) {
            return FileTypeInfo{ "application/x-xz", "XZ" };
        }

        // XML
        if (MatchesMagic(header, bytesRead, kMagicXml)) {
            return FileTypeInfo{ "application/xml", "XML" };
        }

        // Executables
        if (MatchesMagic(header, bytesRead, kMagicElf)) {
            return FileTypeInfo{ "application/x-elf", "ELF" };
        }
        if (MatchesMagic(header, bytesRead, kMagicMz)) {
            return FileTypeInfo{ "application/x-msdownload", "EXE" };
        }

        // WebAssembly
        if (MatchesMagic(header, bytesRead, kMagicWasm)) {
            return FileTypeInfo{ "application/wasm", "WASM" };
        }

        return std::nullopt;
    }

    FileTypeInfo FileTypeDetector::DetectZipContents(const std::filesystem::path& path) const {
        int err = 0;
        zip_t* archive = zip_open(path.string().c_str(), ZIP_RDONLY, &err);
        if (!archive) {
            return { "application/zip", "ZIP" };
        }

        struct ZipGuard {
            zip_t* archive;
            ~ZipGuard() { if (archive) zip_close(archive); }
        } guard{ archive };

        // Check for Office Open XML markers
        const bool hasContentTypes = zip_name_locate(archive, "[Content_Types].xml", 0) >= 0;
        const bool hasWord = zip_name_locate(archive, "word/document.xml", 0) >= 0;
        const bool hasExcel = zip_name_locate(archive, "xl/workbook.xml", 0) >= 0;
        const bool hasPowerpoint = zip_name_locate(archive, "ppt/presentation.xml", 0) >= 0;

        if (hasContentTypes) {
            if (hasWord) {
                return { "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "DOCX" };
            }
            if (hasExcel) {
                return { "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "XLSX" };
            }
            if (hasPowerpoint) {
                return { "application/vnd.openxmlformats-officedocument.presentationml.presentation", "PPTX" };
            }
        }

        // Check for OpenDocument formats
        if (zip_name_locate(archive, "mimetype", 0) >= 0) {
            zip_file_t* mimetypeFile = zip_fopen(archive, "mimetype", 0);
            if (mimetypeFile) {
                char buffer[256] = {};
                const auto bytesRead = zip_fread(mimetypeFile, buffer, sizeof(buffer) - 1);
                zip_fclose(mimetypeFile);

                if (bytesRead > 0) {
                    const std::string_view mimetype(buffer, static_cast<std::size_t>(bytesRead));

                    if (mimetype.find("opendocument.text") != std::string_view::npos) {
                        return { "application/vnd.oasis.opendocument.text", "ODT" };
                    }
                    if (mimetype.find("opendocument.spreadsheet") != std::string_view::npos) {
                        return { "application/vnd.oasis.opendocument.spreadsheet", "ODS" };
                    }
                    if (mimetype.find("opendocument.presentation") != std::string_view::npos) {
                        return { "application/vnd.oasis.opendocument.presentation", "ODP" };
                    }
                }
            }
        }

        // Check for EPUB
        if (zip_name_locate(archive, "META-INF/container.xml", 0) >= 0) {
            return { "application/epub+zip", "EPUB" };
        }

        // Check for JAR/APK
        if (zip_name_locate(archive, "META-INF/MANIFEST.MF", 0) >= 0) {
            if (zip_name_locate(archive, "AndroidManifest.xml", 0) >= 0) {
                return { "application/vnd.android.package-archive", "APK" };
            }
            return { "application/java-archive", "JAR" };
        }

        return { "application/zip", "ZIP" };
    }

    FileTypeInfo FileTypeDetector::DetectOleContents(const std::filesystem::path& path) const {
        // OLE detection confirmed via magic bytes, use extension as hint
        const auto ext = ToLower(path.extension().string());

        if (ext == ".doc") {
            return { "application/msword", "DOC" };
        }
        if (ext == ".xls") {
            return { "application/vnd.ms-excel", "XLS" };
        }
        if (ext == ".ppt") {
            return { "application/vnd.ms-powerpoint", "PPT" };
        }
        if (ext == ".msg") {
            return { "application/vnd.ms-outlook", "MSG" };
        }

        return { "application/x-ole-storage", "OLE" };
    }

} // namespace apu