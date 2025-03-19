#include "t8_utils.h"

namespace t8 {

    void str_ltrim(std::string & str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](uint8_t ch) {
            return !std::isspace(ch);
        }));
    }
    
    void str_rtrim(std::string &str) {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](uint8_t ch) {
            return !std::isspace(ch);
        }).base(), str.end());
    }

    bool str_equals(const std::string & a, const std::string & b, bool ignore_case) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(), [=](char a, char b) {
            return ignore_case ? (std::tolower(static_cast<unsigned char>(a)) ==
                                  std::tolower(static_cast<unsigned char>(b)))
                               : (a == b);
        });
    }

    std::vector<std::string> str_explode(const std::string &text, char token) {
        std::vector<std::string> result;
        result.push_back({});

        for (const auto &ch : text) {
            if (ch == token) {
                result.push_back(std::string());
            } else {
                result.back().push_back(ch);
            }
        }

        return result;
    }
}