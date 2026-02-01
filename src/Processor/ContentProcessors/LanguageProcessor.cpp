#include "LanguageProcessor.hpp"
#include "../../Database/Database.hpp"

#include <unordered_map>
#include <algorithm>

namespace apu {

    namespace {
        // Common words for language detection
        struct LanguageFingerprint {
            std::vector<std::string> commonWords;
            std::vector<std::pair<char, char>> characterRanges;
        };

        const std::unordered_map<std::string, LanguageFingerprint> kLanguageFingerprints = {
            {"English", {
                {"the", "be", "to", "of", "and", "a", "in", "that", "have", "I"},
                {{'A', 'Z'}, {'a', 'z'}}
            }},
            {"Spanish", {
                {"el", "la", "de", "que", "y", "a", "en", "un", "ser", "se"},
                {{'A', 'Z'}, {'a', 'z'}}
            }},
            {"French", {
                {"le", "de", "un", "être", "et", "à", "il", "avoir", "ne", "je"},
                {{'A', 'Z'}, {'a', 'z'}}
            }},
            {"German", {
                {"der", "die", "und", "in", "den", "von", "zu", "das", "mit", "sich"},
                {{'A', 'Z'}, {'a', 'z'}}
            }},
            {"Russian", {
                {"и", "в", "не", "на", "я", "быть", "он", "с", "что", "а"},
                {{static_cast<char>(0xD0), static_cast<char>(0xDF)}}
            }},
            {"Arabic", {
                {"في", "من", "إلى", "على", "هذا", "أن", "كان", "قد", "كل", "ما"},
                {{static_cast<char>(0x80), static_cast<char>(0xFF)}}
            }},
            {"Hebrew", {
                {"של", "את", "ב", "ה", "ל", "על", "כי", "מ", "אם", "לא"},
                {{static_cast<char>(0x80), static_cast<char>(0xFF)}}
            }}
        };

        std::string DetectLanguage(const std::string& text) {
            if (text.empty() || text.length() < 50) {
                return "Unknown";
            }

            // Convert sample to lowercase for word matching
            std::string sample = text.substr(0, std::min<size_t>(1000, text.length()));
            std::transform(sample.begin(), sample.end(), sample.begin(),
                [](unsigned char c) { return std::tolower(c); });

            // Score each language
            std::unordered_map<std::string, int> scores;

            for (const auto& [language, fingerprint] : kLanguageFingerprints) {
                int score = 0;

                // Check for common words
                for (const auto& word : fingerprint.commonWords) {
                    size_t pos = 0;
                    while ((pos = sample.find(word, pos)) != std::string::npos) {
                        // Verify it's a whole word (bounded by spaces or punctuation)
                        bool isWholeWord = true;
                        if (pos > 0 && std::isalnum(static_cast<unsigned char>(sample[pos - 1]))) {
                            isWholeWord = false;
                        }
                        if (pos + word.length() < sample.length() &&
                            std::isalnum(static_cast<unsigned char>(sample[pos + word.length()]))) {
                            isWholeWord = false;
                        }

                        if (isWholeWord) {
                            score += 10;
                        }
                        pos += word.length();
                    }
                }

                scores[language] = score;
            }

            // Find language with highest score
            std::string detectedLanguage = "English"; // Default
            int maxScore = 0;

            for (const auto& [language, score] : scores) {
                if (score > maxScore) {
                    maxScore = score;
                    detectedLanguage = language;
                }
            }

            // If no clear winner, default to English for Latin script
            if (maxScore == 0) {
                // Check if text is primarily Latin characters
                int latinCount = 0;
                for (const unsigned char c : sample) {
                    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                        latinCount++;
                    }
                }

                if (latinCount > static_cast<int>(sample.length()) / 3) {
                    return "English";
                }

                return "Unknown";
            }

            return detectedLanguage;
        }
    }

    LanguageProcessor::LanguageProcessor()
        : IContentProcessor("LanguageProcessor", "Detects what language the content of this document is written in.") {
    }

    AnalysisResult LanguageProcessor::process(std::string uuid) {
        spdlog::info("Detecting language for {}", uuid);

        // For now, returning placeholder - in full implementation, this would:
        // 1. Receive content as parameter or fetch from a shared context
        // 2. Use the detection algorithm above
        // 3. Return language with confidence score

        const std::string detectedLanguage = "English";

        spdlog::info("Detected language for {}: {}", uuid, detectedLanguage);

        return {
            .description = "",
            .interestLevel = InterestLevel::None,
            .language = detectedLanguage
        };
    }

}
