#include "cartridge.h"
#include "core_context.h"

#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

#include "constants.h"

namespace t8::cartridge
{
    static uint8_t hex2num(char c)
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F')
            return 10 + (c - 'A');
        return 0;
    }

    static void parse_block(
        const std::string &pattern,
        const std::string &file_content,
        uint8_t *raw,
        size_t stride,
        size_t rows)
    {
        std::regex line_regex(R"(--\s*(\d+):([0-9a-fA-F]+))");
        std::regex block_regex(pattern);
        std::smatch block_match;

        if (std::regex_search(file_content, block_match, block_regex) && block_match.size() > 1)
        {
            std::stringstream ss(block_match[1].str());
            std::string line;

            while (std::getline(ss, line))
            {
                std::smatch line_match;
                if (std::regex_search(line, line_match, line_regex))
                {
                    int row = std::stoi(line_match[1].str());
                    std::string data_str = line_match[2].str();

                    if (row >= 0 && row < static_cast<int>(rows))
                    {
                        for (size_t i = 0; i < data_str.length(); i += 2)
                        {
                            size_t offset = i / 2;
                            if (offset >= stride)
                                break;
                            uint8_t high = hex2num(data_str[i]);
                            uint8_t low = (i + 1 < data_str.length()) ? hex2num(data_str[i + 1]) : 0;
                            raw[stride * row + offset] = (high << 4) | (low & 0x0F);
                        }
                    }
                }
            }
        }
    }

    bool load(const std::string &name, AppContext *ctx)
    {
        std::ifstream ifs(name, std::ios::in | std::ios::binary);
        std::stringstream buffer;

        if (!ifs.is_open())
            return false;

        buffer << ifs.rdbuf();
        ifs.close();

        auto sprite = std::make_unique<uint8_t[]>(0x2000);
        auto map = std::make_unique<uint8_t[]>(0x4000);
        auto font = std::make_unique<uint8_t[]>(0x800);
        std::string content = buffer.str();

        std::memset(sprite.get(), 0, 0x2000);
        std::memset(map.get(), 0, 0x4000);
        std::memcpy(font.get(), FONT_DATA, 0x800);

        parse_block(R"(--\s*<TILES>([\s\S]*?)--\s*</TILES>)", content, sprite.get(), 64, 128);
        parse_block(R"(--\s*<MAP>([\s\S]*?)--\s*</MAP>)", content, map.get(), 128, 128);
        parse_block(R"(--\s*<FONT>([\s\S]*?)--\s*</FONT>)", content, font.get(), 16, 128);

        std::regex clean_regex(R"(--\s*<(TILES|MAP|FONT)>[\s\S]*?--\s*</\1>\s*)");
        std::string pure_code = std::regex_replace(content, clean_regex, "");
        size_t end = pure_code.find_last_not_of(" \t\n\r\f\v");
        if (end != std::string::npos)
        {
            pure_code = pure_code.substr(0, end + 1);
        }

        std::memcpy(ctx->memory->sprite, sprite.get(), 0x2000);
        std::memcpy(ctx->memory->map, map.get(), 0x4000);
        std::memcpy(ctx->memory->custom_font, font.get(), 0x800);

        ctx->script = pure_code;

        return true;
    }

    static bool is_same_raw(const uint8_t *first, const uint8_t *second, size_t stride)
    {
        for (size_t i = 0; i < stride; i += 1)
        {
            if (first[i] != second[i])
            {
                return false;
            }
        }
        return true;
    }

    static void write_block(
        const std::string &tag,
        std::ofstream &stream,
        const uint8_t *data,
        const uint8_t *example,
        size_t stride,
        size_t rows)
    {
        const static char *hex_digits = "0123456789abcdef";

        if (is_same_raw(data, example, stride * rows))
            return;

        stream << "-- <" << tag << ">\n";
        for (size_t row = 0; row < rows; row += 1)
        {
            const uint8_t *row_ptr = &data[row * stride];
            const uint8_t *exm_ptr = &example[row * stride];

            if (!is_same_raw(row_ptr, exm_ptr, stride))
            {
                stream << "-- " << std::setw(3) << std::setfill('0') << row << ":";
                for (size_t i = 0; i < stride; i += 1)
                {
                    const auto value = row_ptr[i];
                    stream << hex_digits[(value >> 4) & 0x0F];
                    stream << hex_digits[value & 0x0F];
                }
                stream << "\n";
            }
        }
        stream << "-- </" << tag << ">\n";
    }

    bool save(const std::string &name, const AppContext *ctx)
    {
        std::ofstream ofs(name, std::ios::out | std::ios::binary | std::ios::trunc);

        if (!ofs.is_open())
            return false;

        auto empty = std::make_unique<uint8_t[]>(0x4000);
        auto font = std::make_unique<uint8_t[]>(0x800);

        std::memset(empty.get(), 0, 0x4000);
        std::memcpy(font.get(), FONT_DATA, 0x800);

        ofs << ctx->script << "\n\n";

        const auto mem = ctx->memory;

        write_block("TILES", ofs, mem->sprite, empty.get(), 64, 128);
        write_block("MAP", ofs, mem->map, empty.get(), 128, 128);
        write_block("FONT", ofs, mem->custom_font, font.get(), 16, 128);

        return true;
    }
}