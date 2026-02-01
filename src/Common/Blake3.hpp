#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace apu {

    class Blake3 {
    public:
        [[nodiscard]] static std::string HashFile(const std::filesystem::path& path);
        [[nodiscard]] static std::string HashBytes(const void* data, size_t length);
        [[nodiscard]] static std::string HashString(const std::string& data);

    private:
        static constexpr size_t kHashLength = 32;
        static constexpr size_t kBufferSize = 65536; // 64KB chunks
    };

} // namespace apu