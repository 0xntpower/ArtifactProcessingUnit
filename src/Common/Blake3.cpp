#include "../pch.hpp"
#include "Blake3.hpp"

#include <blake3.h>

namespace apu {

    namespace {
        std::string BytesToHex(const uint8_t* bytes, size_t length) {
            std::string hex;
            hex.reserve(length * 2);

            constexpr char hexChars[] = "0123456789abcdef";
            for (size_t i = 0; i < length; ++i) {
                hex.push_back(hexChars[(bytes[i] >> 4) & 0x0F]);
                hex.push_back(hexChars[bytes[i] & 0x0F]);
            }

            return hex;
        }
    }

    std::string Blake3::HashFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file for hashing: " + path.string());
        }

        blake3_hasher hasher;
        blake3_hasher_init(&hasher);

        std::vector<uint8_t> buffer(kBufferSize);
        while (file) {
            file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            const auto bytesRead = file.gcount();
            if (bytesRead > 0) {
                blake3_hasher_update(&hasher, buffer.data(), static_cast<size_t>(bytesRead));
            }
        }

        uint8_t hash[kHashLength];
        blake3_hasher_finalize(&hasher, hash, kHashLength);

        return BytesToHex(hash, kHashLength);
    }

    std::string Blake3::HashBytes(const void* data, size_t length) {
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, data, length);

        uint8_t hash[kHashLength];
        blake3_hasher_finalize(&hasher, hash, kHashLength);

        return BytesToHex(hash, kHashLength);
    }

    std::string Blake3::HashString(const std::string& data) {
        return HashBytes(data.data(), data.size());
    }

} // namespace apu
