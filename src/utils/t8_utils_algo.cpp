#include "t8_utils_algo.h"

#include <array>

#include <algorithm>

namespace t8::utils
{
    std::array<uint32_t, 256> generate_crc_table() noexcept
    {
        auto table = std::array<uint32_t, 256>{};
        std::generate(table.begin(), table.end(), [n = 0]() mutable
                      {
            constexpr uint32_t poly = 0xEDB88320;
            uint32_t t = n++;
            for (int j = 8; j > 0; j--)
                t = (t >> 1) ^ ((t & 1) ? poly : 0);
            return t; });

        return table;
    }

    uint32_t crc32(const uint8_t *data, size_t size)
    {
        static const auto table = generate_crc_table();

        uint32_t crc = 0xFFFFFFFF;
        while (size--)
        {
            crc = ((crc >> 8) & 0x00FFFFFF) ^ table[(crc ^ (*data)) & 0xFF];
            data++;
        }
        return (crc ^ 0xFFFFFFFF);
    }

    uint32_t crc32(const char *data, size_t size)
    {
        return crc32(reinterpret_cast<const uint8_t *>(data), size);
    }

    void str_ltrim(std::string &str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](uint8_t ch)
                                            { return !std::isspace(ch); }));
    }

    void str_rtrim(std::string &str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](uint8_t ch)
                               { return !std::isspace(ch); })
                      .base(),
                  str.end());
    }

    bool str_equals(const std::string &a, const std::string &b, bool ignore_case)
    {
        return std::equal(a.begin(), a.end(), b.begin(), b.end(), [=](char a, char b)
                          { return ignore_case ? (std::tolower(static_cast<unsigned char>(a)) ==
                                                  std::tolower(static_cast<unsigned char>(b)))
                                               : (a == b); });
    }

    std::vector<std::string> str_explode(const std::string &text, char token)
    {
        std::vector<std::string> result;
        result.push_back({});

        for (const auto &ch : text)
        {
            if (ch == token)
            {
                result.push_back(std::string());
            }
            else
            {
                result.back().push_back(ch);
            }
        }

        return result;
    }
}