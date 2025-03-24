#pragma once

#include <fstream>
#include <vector>

namespace t8 {

    template <typename T>
    void byte_reverse(T &val) {
        auto buf = reinterpret_cast<char *>(&val);
        for (auto i = 0; i < (sizeof(T) >> 1); i++) {
            std::swap(buf[i], buf[sizeof(T) - i - 1]);
        }
    }

    class ifstream_t : public std::ifstream {
    private:
        size_t _size;
        bool _use_reverse;

    public:
        ifstream_t(bool use_reverse = false) : _use_reverse(use_reverse) {};

        bool try_open(const std::string &file_name) {
            open(file_name, std::ios::in | std::ios::binary);

            if (good()) {
                seekg(0, std::ios::end);
                _size = tellg();
                seekg(0, std::ios::beg);
                return true;
            }

            return false;
        }

        std::string read_str(size_t size) {
            std::vector<char> result;
            result.resize(size + 1, '\0');
            std::ifstream::read(result.data(), size);
            return result.data();
        }

        template <typename T>
        void read(T *buffer, size_t count = 1) {
            auto raw = reinterpret_cast<char *>(buffer);
            std::ifstream::read(raw, sizeof(T) * count);
            if (_use_reverse) {
                while (count--) {
                    byte_reverse(buffer[count - 1]);
                }
            }
        }

        ~ifstream_t() {
            if (good())
                close();
        }

        const size_t &size() { return _size; }
    };

    class ofstream_t : public std::ofstream {
    private:
        bool _use_reverse;

    public:
        ofstream_t(bool auto_reverse = false) : _use_reverse(auto_reverse) {};

        bool try_open(const std::string &file_name) {
            open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
            return good();
        }

        template <typename T>
        void write(T value) {
            auto raw = reinterpret_cast<char *>(&value);
            if (_use_reverse && sizeof(T) > 1)
                byte_reverse(value);
            std::ofstream::write(raw, sizeof(T));
        }

        template <typename T>
        void write_buf(const T *buffer, size_t count = 1) {
            while (count--) {
                write(*(buffer++));
            }
        }

        ~ofstream_t() {
            if (good())
                close();
        }
    };

}