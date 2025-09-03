#pragma once

#include <algorithm>
#include <random>
#include <stdexcept>

class RandomEngine {
   public:
    static RandomEngine& instance() {
        static RandomEngine inst;
        return inst;
    }

    std::mt19937& getGenerator() { return gen; }

    int getInt(int max) {
        if (max <= 0) return 0;
        std::uniform_int_distribution<int> distrib(0, max);
        return distrib(gen);
    }

    int getInt(int min, int max) {
        std::uniform_int_distribution<int> distrib(min, max);
        return distrib(gen);
    }
    RandomEngine(const RandomEngine&) = delete;
    RandomEngine& operator=(const RandomEngine&) = delete;

   private:
    RandomEngine() : gen(rd()) {}
    std::random_device rd;
    std::mt19937 gen;
};

inline int randomInt(int max) { return RandomEngine::instance().getInt(max); }
template <typename Container>
int randomIndex(const Container& container) {
    if (container.empty()) throw std::runtime_error("Container is empty");
    return randomInt(container.size() - 1);
}

namespace strutils {

inline std::string trim(const std::string& s) {
    using u_char = unsigned char;
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<u_char>(s[start]))) {
        ++start;
    }
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<u_char>(s[end]))) {
        --end;
    }
    return s.substr(start, end - start + 1);
}

inline std::string toUpper(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

inline std::string repeat(std::string_view toRepeat, size_t times) {
    std::string result;
    result.reserve(toRepeat.size() * times);
    for (size_t i = 0; i < times; i++) result += toRepeat;
    return result;
}

inline std::string padLeft(const std::string& s, size_t width, char fill) {
    if (s.size() >= width) return s;
    return std::string(width - s.size(), fill) + s;
}

}  // namespace strutils
