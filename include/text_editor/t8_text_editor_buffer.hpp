#pragma once
#include <cassert>
#include <string>
#include <vector>

namespace t8::text_editor
{
    class GapBuffer
    {
    private:
        static const size_t BASE_SIZE = 64;

        std::vector<char> _buffer;
        size_t _gap_start, _gap_end;

    private:
        void ensure_gap(size_t size)
        {
            if (gap_size() >= size)
                return;

            const auto next_gap_size = size + 32ULL;
            const auto expand_size = next_gap_size - gap_size();
            const auto move_size = _buffer.size() - _gap_end;
            const auto buffer_size = _buffer.size() + expand_size;

            _buffer.resize(buffer_size);
            if (move_size > 0)
            {
                std::memmove(&_buffer[_gap_end + expand_size], &_buffer[_gap_end], move_size);
            }
            _gap_end += expand_size;
        }

        size_t gap_size() const { return _gap_end - _gap_start; }

    public:
        GapBuffer() : _buffer(BASE_SIZE), _gap_start(0), _gap_end(BASE_SIZE) {};

        size_t size() const { return _buffer.size() - gap_size(); }
        size_t cursor() const { return _gap_start; }

        void reset(const std::string &text)
        {
            size_t pos = BASE_SIZE;
            _gap_start = 0;
            _gap_end = BASE_SIZE;

            _buffer.resize(text.size() + BASE_SIZE);
            std::copy(text.begin(), text.end(), _buffer.begin() + BASE_SIZE);
        }

        bool empty() const
        {
            return size() == 0;
        }

        void set_cursor(size_t pos)
        {
            pos = std::min(pos, size());

            if (pos > _gap_start)
            {
                std::memmove(&_buffer[_gap_start], &_buffer[_gap_end], pos - _gap_start);
            }

            if (pos < _gap_start)
            {
                std::memmove(&_buffer[_gap_end - (_gap_start - pos)], &_buffer[pos], _gap_start - pos);
            }

            const auto s = gap_size();
            _gap_start = pos;
            _gap_end = _gap_start + s;
        }

        void insert(char value)
        {
            ensure_gap(1);
            _buffer[_gap_start++] = value;
        }

        void insert(const std::string &text)
        {
            ensure_gap(text.size());
            std::copy(text.begin(), text.end(), _buffer.begin() + _gap_start);
            _gap_start += text.size();
        }

        void clear()
        {
            _gap_start = 0;
            _gap_end = _buffer.size();
        }

        bool erase_before(size_t count = 1)
        {
            count = std::min(count, _gap_start);

            if (count == 0)
                return false;

            _gap_start -= count;
            return count > 0;
        }

        bool erase_after(size_t count = 1)
        {
            count = std::min(count, _buffer.size() - _gap_end);

            if (count == 0)
                return false;

            _gap_end += count;
            return count > 0;
        }

        char operator[](size_t i) const
        {
            assert(i < size());
            return i < _gap_start ? _buffer[i] : _buffer[i + gap_size()];
        }

        std::vector<char> to_buffer() const
        {
            std::vector<char> result;
            result.reserve(size());
            result.insert(result.end(), _buffer.begin(), _buffer.begin() + _gap_start);
            result.insert(result.end(), _buffer.begin() + _gap_end, _buffer.end());
            return result;
        }
    };
}