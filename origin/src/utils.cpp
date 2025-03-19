#include "utils.h"

namespace t8y {

    void utils::ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }));
    }

    void utils::rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
                    return !std::isspace(ch);
                }).base(),
                s.end());
    }

    bool utils::equals(const std::string &a, const std::string &b, bool ignore_case) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(), [=](char a, char b) {
            return ignore_case ? (std::tolower(static_cast<unsigned char>(a)) ==
                                  std::tolower(static_cast<unsigned char>(b)))
                               : (a == b);
        });
    }

    std::vector<std::string> utils::split(const std::string &text, char token) {
        std::vector<std::string> result;
        result.push_back(std::string());

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