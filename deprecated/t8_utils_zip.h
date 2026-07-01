#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace t8::utils
{
    enum class ZipError
    {
        Success,
        FileError,
        InvalidFile,
        InvalidSize,
        InvalidMagic,
        Crc32MisMatch,
        UnsupportZip64,
        UnsupportStream,
        UnsupportMultiDisk,
        UnsupportMethod
    };

    struct ZipFile
    {
        uint16_t version_made;
        uint16_t version_extract;
        uint16_t bitflags;
        uint16_t compression_method;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t modify_time;
        uint16_t modify_date;
        uint32_t crc32;
        uint16_t internal_attributes;
        uint32_t external_attributes;

        std::vector<uint8_t> data;
        std::vector<uint8_t> extra_fields;
        std::vector<uint8_t> file_desc;
        std::string file_name;
        std::string comment;
    };

    class Zipper
    {
    public:
        bool contains(const std::string &file_name);

        const ZipFile &file(const std::string &file_name);

        ZipError open(const std::string &file_name);

        ZipError save(const std::string &file_name);

        void add(const std::string &file_name, const std::vector<uint8_t> &data);

        void add(const std::string &file_name, const std::string &str);

        void remove(const std::string &file_name);

        void set_comment(const std::string &comment);

        const std::string &comment() const;

        uint32_t directory_count() const;

        uint32_t file_count() const;

    private:
        std::string _comment;
        std::unordered_map<std::string, ZipFile> _files;

        uint32_t _directory_count;
        uint32_t _file_count;
    };
}