#pragma once
#include <string>
#include <vector>

namespace t8 {

    void str_ltrim(std::string &s);

    void str_rtrim(std::string &s);

    bool str_equals(const std::string &a, const std::string &b, bool ignore_case = false);
    
    std::vector<std::string> str_explode(const std::string &text, char token);
    
    constexpr bool is_big_endian() {
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01};

        return bint.c[0] != 1;
    }

    template <typename T>
    T min(T v0, T v1) {
        return std::min(v0, v1);
    }

    template <typename T, typename... Args>
    T min(T val, Args... args) {
        return std::min(val, min(args...));
    }

    template <typename T>
    T max(T v0, T v1) {
        return std::max(v0, v1);
    }

    template <typename T, typename... Args>
    T max(T val, Args... args) {
        return std::max(val, max(args...));
    }
    
}