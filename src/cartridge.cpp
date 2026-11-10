#include "cartridge.h"
#include "core_context.h"

#include <fstream>
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
                            uint8_t low = hex2num(data_str[i]);
                            uint8_t high = (i + 1 < data_str.length()) ? hex2num(data_str[i + 1]) : 0;
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

        std::string content = buffer.str();
        VirtualMemory mem;

        std::memset(mem.sprite, 0, 0x2000);
        std::memset(mem.map, 0, 0x4000);
        std::memcpy(mem.custom_font, FONT_DATA, 2048);

        parse_block(R"(--\s*<TILES>([\s\S]*?)--\s*</TILES>)", content, mem.sprite, 64, 128);
        parse_block(R"(--\s*<MAP>([\s\S]*?)--\s*</MAP>)", content, mem.map, 128, 128);
        parse_block(R"(--\s*<FONT>([\s\S]*?)--\s*</FONT>)", content, mem.custom_font, 16, 128);

        std::regex clean_regex(R"(--\s*<(TILES|MAP|FONT)>[\s\S]*?--\s*</\1>\s*)");
        std::string pure_code = std::regex_replace(content, clean_regex, "");
        size_t end = pure_code.find_last_not_of(" \t\n\r\f\v");
        if (end != std::string::npos)
        {
            pure_code = pure_code.substr(0, end + 1);
        }

        std::memcpy(ctx->memory->sprite, mem.sprite, 0x2000);
        std::memcpy(ctx->memory->map, mem.map, 0x4000);
        std::memcpy(ctx->memory->custom_font, mem.custom_font, 0x800);

        ctx->script = pure_code;

        return true;
    }

    bool save(const std::string &name, const AppContext *ctx)
    {

        return false;
    }
}