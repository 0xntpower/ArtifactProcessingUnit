#include "../pch.hpp"
#include "Uuid.hpp"

#include <random>

namespace apu {

    std::string GenerateUuid() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dist;

        const uint64_t high = dist(gen);
        const uint64_t low = dist(gen);

        // UUID v4: set version (4) and variant (RFC 4122) bits
        const uint64_t highFixed = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        const uint64_t lowFixed = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(highFixed >> 32),
            static_cast<uint16_t>(highFixed >> 16),
            static_cast<uint16_t>(highFixed),
            static_cast<uint16_t>(lowFixed >> 48),
            lowFixed & 0xFFFFFFFFFFFFULL);
    }

} // namespace apu