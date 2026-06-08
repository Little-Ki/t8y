#include "t8_algo.h"

#include <array>

#include <algorithm>

namespace t8 {

    void str_ltrim(std::string &str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](uint8_t ch) { return !std::isspace(ch); }));
    }

    void str_rtrim(std::string &str) {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](uint8_t ch) { return !std::isspace(ch); })
                      .base(),
                  str.end());
    }

    bool str_equals(std::string_view a, std::string_view b, bool ignore_case) {
        if (a.size() != b.size())
            return false;

        if (ignore_case) {
            return std::equal(a.begin(), a.end(), b.begin(), [=](char ca, char cb) {
                return std::tolower(static_cast<unsigned char>(ca)) ==
                       std::tolower(static_cast<unsigned char>(cb));
            });
        } else {
            // 🔥 如果不忽略大小写，直接利用标准库极其高效的 memcmp 级别优化
            return a == b;
        }
    }

    std::vector<std::string_view> str_explode(const std::string &text, char token) {
        std::vector<std::string_view> result;
        size_t start = 0;
        size_t end = text.find(token);

        while (end != std::string::npos) {
            result.emplace_back(text.data() + start, end - start);
            start = end + 1;
            end = text.find(token, start);
        }
        result.emplace_back(text.data() + start, text.size() - start);

        return result;
    }
}