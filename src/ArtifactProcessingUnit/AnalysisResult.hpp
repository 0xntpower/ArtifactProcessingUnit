#pragma once

#include <string>

namespace apu {

    enum class InterestLevel {
        None,
        NotInteresting,
        Interesting,
        Critical
    };

    struct AnalysisResult {
        // text analysis
        std::string description;
        InterestLevel interestLevel;
        // language analysis
        std::string language;
    };

    // Helper for serialization
    [[nodiscard]] inline std::string ToString(InterestLevel level) {
        switch (level) {
        case InterestLevel::NotInteresting: return "NotInteresting";
        case InterestLevel::Interesting:    return "Interesting";
        case InterestLevel::Critical:       return "Critical";
        }
        return "Unknown";
    }

} // namespace apu
