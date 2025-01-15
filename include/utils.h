#pragma once

#include <string>
#include <vector>

namespace t8y {
    namespace utils {

        void ltrim(std::string &s);

        void rtrim(std::string &s);

        bool equals(const std::string &a, const std::string &b, bool ignore_case = false);

        std::vector<std::string> split(const std::string &text, char token);

        template <typename T>
        void swapBytes(T &val) {
            constexpr auto SIZE = sizeof(T);
            auto buf = reinterpret_cast<char *>(&val);
            for (auto i = 0; i < (SIZE >> 1); i++) {
                std::swap(buf[i], buf[SIZE - i - 1]);
            }
        }

        template <typename T, typename... Args>
        void swapBytes(T &val, Args &...args) {
            swapBytes(val);
            swapBytes(args...);
        }

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
}