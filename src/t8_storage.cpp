#include "t8_storage.h"

#include "t8_fstream.hpp"
#include "t8_memory.h"
#include "t8_script.h"
#include "t8_utils.h"

#include <algorithm>
#include <fstream>
#include <numeric>

namespace t8 {
    struct CardHeader {
        uint32_t signature;
        uint32_t fh_count;
        uint32_t fh_offset;
    };

    struct FileHeader {
        uint32_t signature;
        uint32_t name;
        uint32_t size;
        uint32_t offset;
    };

    uint32_t find_ch(ifstream_t &in) {
        size_t pos = in.size() - sizeof(CardHeader);
        uint32_t signature = 0;

        do {
            in.seekg(pos, in.beg);
            in.read(&signature);
        } while (signature != 'cart' && pos-- > 0);

        if (signature == 'cart')
            return pos;

        return -1;
    }

    FileHeader read_fh(ifstream_t &in) {
        FileHeader result;
        in.read(&result.signature);
        in.read(&result.size);
        in.read(&result.offset);
        return result;
    }

    CardHeader read_ch(ifstream_t &in) {
        CardHeader result;
        in.read(&result.signature);
        in.read(&result.fh_count);
        in.read(&result.fh_offset);
        return result;
    }

    bool storage_load_cart(const std::string &name) {
        ifstream_t in(is_big_endian());

        if (!in.try_open(name)) {
            return false;
        }

        auto ch_pos = find_ch(in);
        
        if (ch_pos == static_cast<size_t>(-1)) {
            return false;
        }
        
        in.seekg(ch_pos, in.beg);

        auto ch = read_ch(in);

        if (ch.signature != 'cart') {
            return false;
        }

        std::vector<FileHeader> fhs;
        auto count = ch.fh_count;

        in.seekg(ch.fh_offset, in.beg);

        while (count--) {
            auto fh = read_fh(in);
            if (fh.signature != 'ctfh') {
                return false;
            }
            fhs.push_back(fh);
        }

        for (const auto &f : fhs) {
            in.seekg(f.offset, in.beg);

            if (f.name == 'sprt') {
                in.read(mem()->sprite, std::min(0x2000U, f.size));
            }
            if (f.name == 'map') {
                in.read(mem()->map, std::min(0x4000U, f.size));
            }
            if (f.name == 'font') {
                in.read(mem()->custom_font, std::min(0x800U, f.size));
            }
            if (f.name == 'scrp') {
               script_set(in.read_str(f.size));
            }
        }

        return true;
    }

    bool storage_save_cart(const std::string &name) {
        ofstream_t out(is_big_endian());

        if (!out.try_open(name)) {
            return false;
        }

        out.write_buf(mem()->sprite, 0x2000);
        out.write_buf(mem()->map, 0x4000);
        out.write_buf(mem()->custom_font, 0x800);
        out.write_buf(script_get().data(), script_get().size());

        uint32_t offset = 0;

        out.write<uint32_t>('ctfh');
        out.write<uint32_t>('sprt');
        out.write<uint32_t>(0x2000);
        out.write<uint32_t>(offset);
        offset += 0x2000;
        
        out.write<uint32_t>('ctfh');
        out.write<uint32_t>('map');
        out.write<uint32_t>(0x4000);
        out.write<uint32_t>(offset);
        offset += 0x4000;

        out.write<uint32_t>('ctfh');
        out.write<uint32_t>('font');
        out.write<uint32_t>(0x800);
        out.write<uint32_t>(offset);
        offset += 0x800;

        out.write<uint16_t>('ctfh');
        out.write<uint32_t>('font');
        out.write<uint32_t>(script_get().size());
        out.write<uint32_t>(offset);
        
        uint32_t fh_offset = out.tellp();

        out.write<uint32_t>('cart');
        out.write<uint32_t>(4);
        out.write<uint32_t>(fh_offset);
        
        return true;
    }

}