#include "t8_utils_zip.h"
#include "t8_utils_algo.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <type_traits>

namespace t8::utils
{
    constexpr uint32_t MAGIC_CENTRAL_DIRECTORY_FILE_HEADER = 0x02014b50;
    constexpr uint32_t MAGIC_END_OF_CENTRAL_DIRECTORY = 0x06054b50;
    constexpr uint32_t MAGIC_LOCAL_FILE_HEADER = 0x04034b50;
    constexpr uint32_t MAGIC_FILE_DESCRIPTOR = 0x08074b50;

    constexpr uint16_t METHOD_STORE = 0;
    constexpr uint16_t METHOD_DEFLATE = 0x08;

    constexpr uint16_t VERSION_STORE = 0x000A;
    constexpr uint16_t VERSION_DEFLATE = 0x0014;
    constexpr uint16_t VERSION_ZIP64 = 0x002D;

    constexpr uint16_t DATE_NORMAL = 0x21;
    constexpr uint16_t TIME_NORMAL = 0x00;

    struct LocalFileHeader
    {
        uint32_t magic;
        uint16_t version;
        uint16_t bitflags;
        uint16_t compression_method;
        uint16_t modify_time;
        uint16_t modify_date;
        uint32_t crc32;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t filename_length;
        uint16_t extra_length;
    };

    struct CentralDirectoryFileHeader
    {
        uint32_t magic;
        uint16_t version_made;
        uint16_t version_extract;
        uint16_t bitflags;
        uint16_t compression_method;
        uint16_t modify_time;
        uint16_t modify_date;
        uint32_t crc32;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
        uint16_t filename_length;
        uint16_t extra_length;
        uint16_t comment_length;
        uint16_t disk_number;
        uint16_t internal_attributes;
        uint32_t external_attributes;
        uint32_t file_offset;
    };

    struct EndOfCentralDirectoryRecord
    {
        uint32_t magic;
        uint16_t disk_number;
        uint16_t directory_disk_number;
        uint16_t directory_entries;
        uint16_t directory_total_entires;
        uint32_t directory_size;
        uint32_t directory_offset;
        uint16_t comment_length;
    };

    struct LocalFileDescriptor
    {
        uint32_t magic;
        uint32_t crc32;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
    };

    template <typename T>
    void byte_reverse(T &val)
    {
        static_assert(std::is_trivial_v<T>);
        static_assert(std::is_standard_layout_v<T>);
        static_assert(!std::is_class_v<T>);
        auto buf = reinterpret_cast<char *>(&val);
        for (auto i = 0; i < (sizeof(T) >> 1); i++)
        {
            std::swap(buf[i], buf[sizeof(T) - i - 1]);
        }
    }

    class ifstream_t
    {
    private:
        std::ifstream _stream;
        size_t _size;
        bool _reverse;

    public:
        ifstream_t() : _reverse(is_big_endian()) {};

        bool try_open(const std::string &file_name)
        {
            _stream.open(file_name, std::ios::in | std::ios::binary);

            if (_stream.good())
            {
                _stream.seekg(0, std::ios::end);
                _size = _stream.tellg();
                _stream.seekg(0, std::ios::beg);
                return true;
            }

            return false;
        }

        std::string read_str(size_t size)
        {
            std::string result;
            result.resize(size);
            _stream.read(&result[0], size);
            return result;
        }

        void read_buf(char *buf, size_t size)
        {
            _stream.read(buf, size);
        }

        void read_buf(uint8_t *buf, size_t size)
        {
            _stream.read(reinterpret_cast<char *>(buf), size);
        }

        void skip(size_t size)
        {
            _stream.ignore(size);
        }

        template <typename T>
        void read(T &val)
        {
            _stream.read(reinterpret_cast<char *>(&val), sizeof(T));
            if (_reverse)
            {
                byte_reverse(val);
            }
        }

        ~ifstream_t()
        {
            if (_stream.is_open())
                _stream.close();
        }

        size_t size() const { return _size; }

        void seek(size_t pos) { _stream.seekg(pos, std::ios::beg); }

        size_t pos() { return _stream.tellg(); }
    };

    class ofstream_t
    {
    private:
        std::ofstream _stream;
        bool _reverse;

    public:
        ofstream_t() : _reverse(is_big_endian()) {};

        bool try_open(const std::string &file_name)
        {
            _stream.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
            return _stream.good();
        }

        void write_buf(const uint8_t *val, size_t size)
        {
            _stream.write(reinterpret_cast<const char *>(val), size);
        }

        void write_buf(const char *val, size_t size)
        {
            _stream.write(val, size);
        }

        template <typename T>
        void write(T val)
        {
            if (_reverse)
            {
                byte_reverse(val);
            }
            _stream.write(reinterpret_cast<char *>(&val), sizeof(T));
        }

        size_t pos() { return _stream.tellp(); }

        ~ofstream_t()
        {
            if (_stream.is_open())
                _stream.close();
        }
    };

    EndOfCentralDirectoryRecord read_end_of_central_directory(ifstream_t &in)
    {
        EndOfCentralDirectoryRecord result{0};
        in.read(result.magic);
        in.read(result.disk_number);
        in.read(result.directory_disk_number);
        in.read(result.directory_entries);
        in.read(result.directory_total_entires);
        in.read(result.directory_size);
        in.read(result.directory_offset);
        in.read(result.comment_length);
        return result;
    }

    CentralDirectoryFileHeader read_central_directory_file_header(ifstream_t &in)
    {
        CentralDirectoryFileHeader result{0};
        in.read(result.magic);
        in.read(result.version_made);
        in.read(result.version_extract);
        in.read(result.bitflags);
        in.read(result.compression_method);
        in.read(result.modify_time);
        in.read(result.modify_date);
        in.read(result.crc32);
        in.read(result.compressed_size);
        in.read(result.uncompressed_size);
        in.read(result.filename_length);
        in.read(result.extra_length);
        in.read(result.comment_length);
        in.read(result.disk_number);
        in.read(result.internal_attributes);
        in.read(result.external_attributes);
        in.read(result.file_offset);
        return result;
    }

    LocalFileHeader read_local_file_header(ifstream_t &in)
    {
        LocalFileHeader result{0};
        in.read(result.magic);
        in.read(result.version);
        in.read(result.bitflags);
        in.read(result.compression_method);
        in.read(result.modify_time);
        in.read(result.modify_date);
        in.read(result.crc32);
        in.read(result.compressed_size);
        in.read(result.uncompressed_size);
        in.read(result.filename_length);
        in.read(result.extra_length);
        return result;
    }

    void write_local_file_header(ofstream_t &out, const ZipFile &file)
    {
        out.write(MAGIC_LOCAL_FILE_HEADER);
        out.write(file.version_made);
        out.write(file.bitflags);
        out.write(file.compression_method);
        out.write(file.modify_time);
        out.write(file.modify_date);
        out.write(file.crc32);
        out.write(file.compressed_size);
        out.write(file.uncompressed_size);
        out.write(static_cast<uint16_t>(file.file_name.size()));
        out.write(static_cast<uint16_t>(file.extra_fields.size()));
        if (file.file_name.size())
            out.write_buf(file.file_name.data(), file.file_name.size());
        if (file.extra_fields.size())
            out.write_buf(file.extra_fields.data(), file.extra_fields.size());
        if (file.data.size())
            out.write_buf(file.data.data(), file.data.size());
    }

    void write_central_directory_file_header(ofstream_t &out, const ZipFile &file, uint32_t file_offset)
    {
        out.write(MAGIC_CENTRAL_DIRECTORY_FILE_HEADER);
        out.write(file.version_made);
        out.write(file.version_extract);
        out.write(file.bitflags);
        out.write(file.compression_method);
        out.write(file.modify_time);
        out.write(file.modify_date);
        out.write(file.crc32);
        out.write(file.compressed_size);
        out.write(file.uncompressed_size);
        out.write(static_cast<uint16_t>(file.file_name.size()));
        out.write(static_cast<uint16_t>(file.extra_fields.size()));
        out.write(static_cast<uint16_t>(file.comment.size()));
        out.write(static_cast<uint16_t>(0));
        out.write(file.internal_attributes);
        out.write(file.external_attributes);
        out.write(file_offset);
        out.write_buf(file.file_name.data(), file.file_name.size());
    }

    bool find_end_of_central_directory(ifstream_t &in, size_t &pos)
    {
        size_t start = in.size() >= 64 ? in.size() - 64 : 0;
        size_t end = in.size();
        uint32_t signature = 0;
        uint32_t find = MAGIC_END_OF_CENTRAL_DIRECTORY;

        for (pos = start; pos < end; pos++)
        {
            in.seek(pos);
            in.read(signature);
            if (signature == MAGIC_END_OF_CENTRAL_DIRECTORY)
            {
                return true;
            }
        }

        return false;
    }

    bool Zipper::contains(const std::string &file_name)
    {
        return _files.find(file_name) != _files.end();
    }

    const ZipFile &Zipper::file(const std::string &file_name)
    {
        return _files[file_name];
    }

    ZipError Zipper::open(const std::string &file_name)
    {
        ifstream_t in;

        uint32_t directory_count = 0;
        uint32_t file_count = 0;

        if (!in.try_open(file_name))
        {
            return ZipError::FileError;
        }

        if (in.size() < 22)
        {
            return ZipError::InvalidSize;
        }

        size_t eocd_pos = 0;
        if (!find_end_of_central_directory(in, eocd_pos))
        {
            return ZipError::InvalidFile;
        }

        in.seek(eocd_pos);
        auto eocd = read_end_of_central_directory(in);

        if (eocd.directory_total_entires != eocd.directory_entries)
        {
            return ZipError::UnsupportMultiDisk;
        }

        if (eocd.magic != MAGIC_END_OF_CENTRAL_DIRECTORY)
        {
            return ZipError::InvalidMagic;
        }

        if (eocd.comment_length > 0)
        {
            _comment = in.read_str(eocd.comment_length);
        }

        auto count = eocd.directory_entries;

        in.seek(eocd.directory_offset);

        for (auto i = 0; i < count; i += 1)
        {
            ZipFile file;
            auto cdfh = read_central_directory_file_header(in);

            if (cdfh.magic != MAGIC_CENTRAL_DIRECTORY_FILE_HEADER)
            {
                return ZipError::InvalidMagic;
            }

            if (cdfh.bitflags & 0x8)
            {
                return ZipError::UnsupportStream;
            }

            if (cdfh.compression_method != METHOD_STORE)
            {
                return ZipError::UnsupportMethod;
            }

            if (cdfh.filename_length)
            {
                file.file_name = in.read_str(cdfh.filename_length);
            }

            if (cdfh.extra_length)
            {
                file.extra_fields.resize(cdfh.extra_length);
                in.read_buf(file.extra_fields.data(), cdfh.extra_length);
            }

            size_t next = in.pos();

            in.seek(cdfh.file_offset);
            auto fh = read_local_file_header(in);

            if (cdfh.file_offset + fh.filename_length + fh.extra_length + cdfh.compressed_size > in.size())
                return ZipError::InvalidFile;

            if (fh.magic != MAGIC_LOCAL_FILE_HEADER)
            {
                return ZipError::InvalidMagic;
            }

            if (*(file.file_name.end() - 1) == '/')
            {
                directory_count += 1;
            }
            else
            {
                file_count += 1;
                if (cdfh.compressed_size)
                {
                    file.data.resize(cdfh.compressed_size);
                    in.skip(fh.filename_length + fh.extra_length);
                    in.read_buf(file.data.data(), cdfh.compressed_size);
                }
            }

            file.version_made = cdfh.version_made;
            file.version_extract = cdfh.version_extract;
            file.bitflags = cdfh.bitflags;
            file.compression_method = cdfh.compression_method;
            file.compressed_size = cdfh.compressed_size;
            file.uncompressed_size = cdfh.uncompressed_size;
            file.modify_time = cdfh.modify_time;
            file.modify_date = cdfh.modify_date;
            file.crc32 = cdfh.crc32;
            file.internal_attributes = cdfh.internal_attributes;
            file.external_attributes = cdfh.external_attributes;

            _files.emplace(file.file_name, file);

            in.seek(next);
        }

        _directory_count = directory_count;
        _file_count = file_count;

        return ZipError::Success;
    }

    ZipError Zipper::save(const std::string &file_name)
    {
        ofstream_t out;

        if (!out.try_open(file_name))
        {
            return ZipError::FileError;
        }

        std::vector<ZipFile *> files;
        std::vector<size_t> offsets;

        for (auto &f : _files)
        {
            files.push_back(&(f.second));
            offsets.push_back(out.pos());
            write_local_file_header(out, f.second);
        }

        size_t cdfh_offset = out.pos();

        for (auto i = 0; i < files.size(); i++)
        {
            write_central_directory_file_header(out, *files[i], static_cast<uint16_t>(offsets[i]));
        }

        size_t cdfh_end = out.pos();
        size_t cdfh_size = cdfh_end - cdfh_offset;

        out.write(MAGIC_END_OF_CENTRAL_DIRECTORY);
        out.write<uint16_t>(0);
        out.write<uint16_t>(0);
        out.write(static_cast<uint16_t>(files.size()));
        out.write(static_cast<uint16_t>(files.size()));
        out.write(static_cast<uint32_t>(cdfh_size));
        out.write(static_cast<uint32_t>(cdfh_offset));
        out.write(static_cast<uint16_t>(_comment.size()));
        if (_comment.size())
            out.write_buf(_comment.data(), _comment.size());

        return ZipError::Success;
    }

    void Zipper::add(const std::string &file_name, const std::vector<uint8_t> &data)
    {
        ZipFile file;

        file.version_made = VERSION_STORE;
        file.version_extract = VERSION_STORE;
        file.bitflags = 0;
        file.compression_method = METHOD_STORE;
        file.compressed_size = static_cast<uint32_t>(data.size());
        file.uncompressed_size = static_cast<uint32_t>(data.size());
        file.modify_date = DATE_NORMAL;
        file.modify_time = TIME_NORMAL;
        file.crc32 = crc32(data.data(), data.size());
        file.internal_attributes = 0;
        file.external_attributes = 32;

        file.file_name = file_name;
        file.data = data;

        _files.emplace(file_name, file);
    }

    void Zipper::add(const std::string &file_name, const std::string &str)
    {
        std::vector<uint8_t> data(str.size());
        std::copy(str.begin(), str.end(), data.begin());
        add(file_name, data);
    }

    void Zipper::remove(const std::string &file_name)
    {
        _files.erase(file_name);
    }

    const std::string &Zipper::comment() const
    {
        return _comment;
    }

    uint32_t Zipper::file_count() const
    {
        return _file_count;
    }

    uint32_t Zipper::directory_count() const
    {
        return _directory_count;
    }

    void Zipper::set_comment(const std::string &comment)
    {
        _comment = comment;
    }
}