#pragma once
#include <string>
#include <stack>
#include <cassert>
#include <vector>

namespace t8::ted
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

    enum class EditType
    {
        Insert,
        Delete
    };

    struct EditRecord
    {
        EditType type;
        size_t pos;
        bool backspace;
        std::string text;
    };

    class EditRecorder
    {
    private:
        std::stack<EditRecord> _undos;
        std::stack<EditRecord> _redos;

        bool _merging = true;

    public:
        bool can_undo() const { return !_undos.empty(); }
        bool can_redo() const { return !_redos.empty(); }

        EditRecord pop_undo()
        {
            auto e = _undos.top();
            _undos.pop();
            return e;
        }

        EditRecord pop_redo()
        {
            auto e = _redos.top();
            _redos.pop();
            return e;
        }

        void break_merge()
        {
            _merging = false;
        }

        void clear_redo()
        {
            while (!_redos.empty())
                _redos.pop();
        }

        void clear_undo()
        {
            while (!_undos.empty())
                _undos.pop();
        }

        void push_redo(EditRecord e)
        {
            _redos.push(std::move(e));
        }

        void push_undo(EditRecord e, bool from_redo = false)
        {
            if (!from_redo)
                clear_redo();

            if (!_merging || _undos.empty())
            {
                _undos.push(std::move(e));
                _merging = true;
                return;
            }

            auto &o = _undos.top();

            if (o.type != e.type)
            {
                _undos.push(std::move(e));
                return;
            }

            if (o.type == EditType::Insert)
            {
                if (e.pos == o.text.size() + o.pos)
                {
                    o.text.append(e.text);
                }
                else
                {
                    _undos.push(std::move(e));
                }
            }

            if (o.type == EditType::Delete)
            {
                if (o.backspace != e.backspace)
                {
                    _undos.push(std::move(e));
                }
                else if (e.backspace && e.pos == o.pos - o.text.size())
                {
                    o.text.insert(0, e.text);
                }
                else if (!e.backspace && e.pos == o.pos)
                {
                    o.text.append(e.text);
                }
                else
                {
                    _undos.push(std::move(e));
                }
            }
        }
    };

    struct EditSelection
    {
        size_t anchor = 0;
        size_t cursor = 0;

        bool empty() const
        {
            return anchor == cursor;
        }

        size_t start() const
        {
            return std::min(anchor, cursor);
        }

        size_t end() const
        {
            return std::max(anchor, cursor);
        }

        size_t size() const
        {
            return end() - start();
        }

        void collapse(size_t pos)
        {
            anchor = cursor = pos;
        }
    };

    class Ted
    {
    private:
        GapBuffer _buffer;
        EditSelection _selection;
        EditRecorder _recorder;
        std::vector<size_t> _line_starts;

        size_t clamp(size_t pos) const
        {
            return std::min(pos, _buffer.size());
        }

        void update_line_starts()
        {
            _line_starts.clear();
            _line_starts.push_back(0);

            for (auto i = 0; i < _buffer.size(); i++)
            {
                if (_buffer[i] == '\n')
                {
                    _line_starts.push_back(i + 1);
                }
            }
        }

        void update_lines_before_erase(size_t start, size_t end)
        {
            const auto first = std::upper_bound(_line_starts.begin(), _line_starts.end(), start);
            const auto last = std::upper_bound(_line_starts.begin(), _line_starts.end(), end);
            const auto count = end - start;
            auto it = (first != last) ? _line_starts.erase(first, last) : first;

            while (it != _line_starts.end())
            {
                *it -= count;
                it += 1;
            }

            if (_line_starts.empty())
            {
                _line_starts.push_back(0);
            }
        }

        void update_lines_before_insert(size_t pos, const std::string &text)
        {
            if (text.empty())
                return;

            auto it = std::upper_bound(_line_starts.begin(), _line_starts.end(), pos);

            for (auto i = 0; i < text.size(); i += 1)
            {
                if (text[i] == '\n')
                {
                    it = _line_starts.insert(it, pos + i + 1);
                    it += 1;
                }
            }

            while (it != _line_starts.end())
            {
                *it += text.size();
                it += 1;
            }
        }

        std::string get_selection() const
        {
            if (_selection.empty())
                return "";

            std::string result;
            result.reserve(_selection.size());

            for (size_t i = _selection.start(); i < _selection.end(); ++i)
            {
                result.push_back(_buffer[i]);
            }
            return result;
        }

        bool delete_selection()
        {
            if (_selection.empty())
                return false;

            _recorder.break_merge();

            update_lines_before_erase(_selection.start(), _selection.end());
            _recorder.push_undo({EditType::Delete,
                                 _selection.cursor,
                                 _selection.cursor >= _selection.anchor,
                                 get_selection()});
            _recorder.break_merge();

            _buffer.set_cursor(_selection.end());
            _buffer.erase_before(_selection.size());

            _selection.collapse(_buffer.cursor());

            return true;
        }

    public:
        void insert(const std::string &text)
        {
            delete_selection();
            const auto cursor = _buffer.cursor();
            update_lines_before_insert(cursor, text);
            _recorder.push_undo({EditType::Insert,
                                 _buffer.cursor(),
                                 false,
                                 text});
            _buffer.insert(text);
            _selection.collapse(_buffer.cursor());
        }

        void insert(char c)
        {
            insert(std::string(1, c));
        }

        void erase_before()
        {
            if (!delete_selection() && _buffer.cursor() > 0)
            {
                update_lines_before_erase(_buffer.cursor() - 1, _buffer.cursor());
                _recorder.push_undo({EditType::Delete,
                                     _buffer.cursor(),
                                     true,
                                     std::string(1, _buffer[_buffer.cursor() - 1])});
                _buffer.erase_before(1);
            }

            _selection.collapse(_buffer.cursor());
        }

        void erase_after()
        {
            if (!delete_selection() && _buffer.cursor() < _buffer.size())
            {
                update_lines_before_erase(_buffer.cursor(), _buffer.cursor() + 1);
                _recorder.push_undo({EditType::Delete,
                                     _buffer.cursor(),
                                     false,
                                     std::string(1, _buffer[_buffer.cursor()])});
                _buffer.erase_after(1);
            }

            _selection.collapse(_buffer.cursor());
        }

        bool can_undo()
        {
            return _recorder.can_undo();
        }

        bool can_redo()
        {
            return _recorder.can_redo();
        }

        void undo()
        {
            if (_recorder.can_undo())
            {
                const auto &o = _recorder.pop_undo();
                _recorder.push_redo(o);

                if (o.type == EditType::Delete)
                {
                    if (o.backspace)
                    {
                        update_lines_before_insert(o.pos - o.text.size(), o.text);
                        _buffer.set_cursor(o.pos - o.text.size());
                        _buffer.insert(o.text);
                    }
                    else
                    {
                        update_lines_before_insert(o.pos, o.text);
                        _buffer.set_cursor(o.pos);
                        _buffer.insert(o.text);
                        _buffer.set_cursor(o.pos);
                    }
                }

                if (o.type == EditType::Insert)
                {
                    update_lines_before_erase(o.pos, o.pos + o.text.size());
                    _buffer.set_cursor(o.pos);
                    _buffer.erase_after(o.text.size());
                }

                _recorder.break_merge();
            }

            collapse(_buffer.cursor());
        }

        void redo()
        {
            if (_recorder.can_redo())
            {
                const auto &o = _recorder.pop_redo();
                _recorder.push_undo(o, true);

                if (o.type == EditType::Delete)
                {
                    if (o.backspace)
                    {
                        update_lines_before_erase(o.pos - o.text.size(), o.pos);
                        _buffer.set_cursor(o.pos);
                        _buffer.erase_before(o.text.size());
                    }
                    else
                    {
                        update_lines_before_erase(o.pos, o.pos + o.text.size());
                        _buffer.set_cursor(o.pos);
                        _buffer.erase_after(o.text.size());
                    }
                }

                if (o.type == EditType::Insert)
                {
                    update_lines_before_insert(o.pos, o.text);
                    _buffer.set_cursor(o.pos);
                    _buffer.insert(o.text);
                    _buffer.set_cursor(o.pos + o.text.size());
                }

                _recorder.break_merge();
            }

            collapse(_buffer.cursor());
        }

        const auto &buffer() const { return _buffer; }

        void reset(const std::string &text)
        {
            std::string t;
            t.reserve(text.size());

            for (auto i = 0; i < text.size(); i++)
            {
                if (text[i] == '\r')
                {
                    t.push_back('\n');

                    if (i < text.size() - 1 && text[i + 1] == '\n')
                    {
                        i += 1;
                    }
                }
                else
                {
                    t.push_back(text[i]);
                }
            }

            _buffer.reset(t);
            _buffer.set_cursor(0);
            _selection.collapse(0);
            _recorder.clear_redo();
            _recorder.clear_undo();
            _recorder.break_merge();

            update_line_starts();
        }

        void set_anchor(size_t pos)
        {
            _selection.anchor = clamp(pos);
            _recorder.break_merge();
        }

        void set_cursor(size_t pos)
        {
            _selection.cursor = clamp(pos);
            _buffer.set_cursor(pos);
            _recorder.break_merge();
        }

        size_t anchor() const
        {
            return _selection.anchor;
        }

        size_t cursor() const
        {
            return _selection.cursor;
        }

        size_t sel_start() const
        {
            return _selection.start();
        }

        size_t sel_end() const
        {
            return _selection.end();
        }

        void collapse(size_t pos)
        {
            _selection.collapse(clamp(pos));
            _buffer.set_cursor(pos);
            _recorder.break_merge();
        }

        void select_all()
        {
            set_anchor(0);
            set_cursor(_buffer.size());
        }

        size_t line_start(size_t i) const
        {
            assert(i <= _line_starts.size());
            return i < _line_starts.size() ? _line_starts[i] : _buffer.size();
        }

        size_t line_size(size_t i) const
        {
            const auto j = line_start(i);
            const auto k = line_start(i + 1);
            return k - j;
        }

        size_t line_count() const
        {
            return _line_starts.size();
        }

        size_t line_index(size_t pos) const
        {
            const auto it = std::upper_bound(_line_starts.begin(), _line_starts.end(), pos);
            return it - 1 - _line_starts.begin();
        }

        size_t size() const
        {
            return _buffer.size();
        }

        std::string to_string()
        {
            const auto buffer = _buffer.to_buffer();
            return std::string(buffer.begin(), buffer.end());
        }

        char get_char(size_t line, size_t column)
        {
            const auto i = line_start(line);
            const auto j = line_start(line + 1);
            return (i + column < j) ? _buffer[i + column] : '\0';
        }

        char get_char(size_t index)
        {
            return (index < _buffer.size()) ? _buffer[index] : '\0';
        }
    };

}