#include "t8_storage.h"

#include "t8_memory.h"
#include "t8_script.h"
#include "t8_utils.h"

#include <algorithm>
#include <fstream>

namespace t8 {
    struct CardFileHeader {
        uint16_t signature;
        uint8_t big_endian;
        uint8_t meta_count;
    };

    enum class MetaType : uint8_t {
        Unknown = 0,
        Sprite = 1,
        Map = 2,
        Font = 3,
        Script = 4
    };

    struct CartMeta {
        MetaType type;
        size_t size;
    };

    class FReader : public std::ifstream {
    private:
        size_t _file_size;
        size_t _offset;

    public:
        bool try_open(const std::string &name) {
            open(name, std::ios::binary | std::ios::in);

            seekg(end);
            _file_size = tellg();
            seekg(beg);

            return good();
        }

        template <typename T>
        bool try_read(T *out, size_t size = sizeof(T)) {
            if (_offset + size > _file_size)
                return false;
            read(reinterpret_cast<char *>(out), size);
            return true;
        }

        const size_t &file_size() { return _file_size; }

        ~FReader() {
            if (good())
                close();
        }
    };

    class FWriter : public std::ofstream {
    public:
        bool try_open(const std::string &name) {
            open(name, std::ios::binary | std::ios::out | std::ios::trunc);
            return good();
        }

        template <typename T>
        bool try_write(const T *data, size_t size = sizeof(T)) {
            write(reinterpret_cast<const char *>(data), size);
            return true;
        }

        ~FWriter() {
            if (good())
                close();
        }
    };

    bool storage_load_cart(const std::string &name) {
        FReader in;
        if (!in.try_open(name)) {
            return false;
        }

        if (in.file_size() < sizeof(CardFileHeader)) {
            return false;
        }

        CardFileHeader header;

        if (!in.try_read(&header)) {
            return false;
        };

        if (header.big_endian != is_big_endian()) {
            byte_reverse(header.signature);
        }

        if (header.signature != 't8') {
            return false;
        }

        std::vector<CartMeta> metas;

        while (header.meta_count--) {
            CartMeta meta;

            if (!in.try_read(&meta)) {
                return false;
            }

            if (header.big_endian != is_big_endian()) {
                byte_reverse(meta.size);
            }

            metas.push_back(meta);
        }

        auto data_size = 0;

        for (const auto &i : metas) {
            data_size += i.size;
        }

        if (in.file_size() < sizeof(CardFileHeader) + sizeof(CartMeta) * header.meta_count + data_size) {
            return false;
        }

        for (const auto &i : metas) {
            if (i.type == MetaType::Sprite) {
                if (!in.try_read(mem()->sprite, std::min(0x2000ULL, i.size))) {
                    return false;
                }
            }

            if (i.type == MetaType::Script) {
                std::vector<char> buffer(i.size);
                if (!in.try_read(buffer.data(), i.size)) {
                    return false;
                }
                script_set(buffer.data());
            }

            if (i.type == MetaType::Map) {
                if (!in.try_read(mem()->map, std::min(0x4000ULL, i.size))) {
                    return false;
                }
            }

            if (i.type == MetaType::Font) {
                if (!in.try_read(mem()->custom_font, std::min(0x800ULL, i.size))) {
                    return false;
                }
            }
        }

        return true;
    }

    bool storage_save_cart(const std::string &name, std::string *err) {
        CardFileHeader header;
        header.big_endian = is_big_endian();
        header.meta_count = 4;
        header.signature = 't8';

        FWriter out;

        if (!out.try_open(name)) {
            return false;
        }

        out.try_write(&header);

        CartMeta meta;

        meta.type = MetaType::Sprite;
        meta.size = 0x2000;
        out.try_write(&meta);
        meta.type = MetaType::Font;
        meta.size = 0x800;
        out.try_write(&meta);
        meta.type = MetaType::Map;
        meta.size = 0x4000;
        out.try_write(&meta);
        meta.type = MetaType::Script;
        meta.size = script_get().size();
        out.try_write(&meta);

        
        out.try_write(mem()->sprite, 0x2000);
        out.try_write(mem()->custom_font, 0x800);
        out.try_write(mem()->map, 0x4000);
        out.try_write(script_get().data(), script_get().size());

        return true;
    }

}