#pragma once
#include <cassert>
#include <string>
#include <vector>

namespace t8::text_editor {
    class GapBuffer {
    private:
        static const size_t BASE_SIZE = 64;

        std::vector<char> _buffer;
        size_t _gap_start, _gap_end;

        std::vector<size_t> line_starts{0};

    private:
        void ensure_gap(size_t n) {
            const auto current_gap_size = gap_size();

            if (current_gap_size >= n)
                return;

            const auto next_gap_size = n + 32ULL;
            const auto expand_size = next_gap_size - current_gap_size;
            const auto move_size = _buffer.size() - _gap_end;
            const auto buffer_size = _buffer.size() + expand_size;

            _buffer.resize(buffer_size);

            if (buffer_size > 0) {
                std::memmove(&_buffer[_gap_end + expand_size], &_buffer[_gap_end], move_size);
            }

            _gap_end += expand_size;
        }

        size_t gap_size() const { return _gap_end - _gap_start; }

    public:
        GapBuffer() : _buffer(BASE_SIZE), _gap_start(0), _gap_end(BASE_SIZE) {};

        size_t size() const { return _buffer.size() - gap_size(); }
        size_t cursor() const { return _gap_start; }

        void reset(const std::string &text) {
            size_t pos = BASE_SIZE;
            _gap_start = 0;
            _gap_end = BASE_SIZE;
            _buffer.resize(text.size() + BASE_SIZE);
            std::copy(text.begin(), text.end(), _buffer.begin() + BASE_SIZE);

            line_starts.clear();
            line_starts.push_back(0);

            for (auto i = 0; i < text.size(); i += 1) {
                if (text[i] == '\n') {
                    line_starts.push_back(i + 1);
                }
            }
        }

        bool empty() const {
            return size() == 0;
        }

        void set_cursor(size_t pos) {
            pos = std::min(pos, size());

            if (pos > _gap_start) {
                std::memmove(&_buffer[_gap_start], &_buffer[_gap_end], pos - _gap_start);
            }

            if (pos < _gap_start) {
                std::memmove(&_buffer[_gap_end - (_gap_start - pos)], &_buffer[pos], _gap_start - pos);
            }

            const auto s = gap_size();
            _gap_start = pos;
            _gap_end = _gap_start + s;
        }

        void insert(char value) {
            ensure_gap(1);

            auto it = std::upper_bound(
                line_starts.begin(),
                line_starts.end(),
                _gap_start);

            if (value == '\n') {
                it = line_starts.insert(it, _gap_start + 1);
                it += 1;
            }

            for (; it != line_starts.end(); it += 1) {
                *it += 1;
            }

            _buffer[_gap_start++] = value;
        }

        void insert(const std::string &text) {
            ensure_gap(text.size());

            auto it = std::upper_bound(
                line_starts.begin(),
                line_starts.end(),
                _gap_start);

            for (auto i = 0; i < text.size(); i += 1) {
                if (text[i] == '\n') {
                    it = line_starts.insert(it, _gap_start + i + 1);
                    it += 1;
                }
            }

            for (; it != line_starts.end(); it += 1) {
                *it += text.size();
            }

            std::copy(text.begin(), text.end(), _buffer.begin() + _gap_start);
            _gap_start += text.size();
        }

        void clear() {
            _gap_start = 0;
            _gap_end = _buffer.size();
        }

        bool erase_before(size_t count = 1) {
            count = std::min(count, _gap_start);

            if (count == 0)
                return false;

            auto it = std::lower_bound(
                line_starts.begin(),
                line_starts.end(),
                _gap_start);

            for (auto i = _gap_start - count; i < _gap_start; i += 1) {
                if (_buffer[i] == '\n') {
                    it = line_starts.erase(it);
                }
            }

            for (; it != line_starts.end(); it += 1) {
                *it -= count;
            }

            _gap_start -= count;
            return count > 0;
        }

        bool erase_after(size_t count = 1) {
            count = std::min(count, size() - _gap_end);

            if (count == 0)
                return false;

            auto it = std::upper_bound(
                line_starts.begin(),
                line_starts.end(),
                _gap_start);

            for (auto i = _gap_end; i < _gap_end + count; i += 1) {
                if (_buffer[i] == '\n') {
                    it = line_starts.erase(it);
                }
            }

            for (; it != line_starts.end(); it += 1) {
                *it += count;
            }

            _gap_end += count;

            return count > 0;
        }

        char operator[](size_t i) const {
            assert(i < size());
            return i < _gap_start ? _buffer[i] : _buffer[i + gap_size()];
        }

        size_t line_start(size_t i) const {
            assert(i <= line_starts.size());
            return i < line_starts.size() ? line_starts[i] : size();
        }

        size_t line_size(size_t i) const {
            const auto j = line_start(i);
            const auto k = line_start(i + 1);
            return k - j;
        }

        size_t line_count() const {
            return line_starts.size();
        }

        std::pair<size_t, size_t> index_line(size_t index) const {
            auto it = std::lower_bound(
                line_starts.begin(),
                line_starts.end(),
                index);
            return std::make_pair(it - line_starts.begin(), *it);
        }

        std::vector<char> to_buffer() const {
            std::vector<char> result;
            result.reserve(size());
            result.insert(result.end(), _buffer.begin(), _buffer.begin() + _gap_start);
            result.insert(result.end(), _buffer.begin() + _gap_end, _buffer.end());
            return result;
        }
    };
}