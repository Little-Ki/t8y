#pragma once
#include "t8_text_editor_buffer.hpp"
#include "t8_text_editor_record.hpp"
#include "t8_text_editor_selection.hpp"
#include <cmath>

namespace t8::text_editor {

    class TextEditor {
    private:
        GapBuffer _buffer;
        EditSelection _selection;
        EditRecorder _recorder;
        std::vector<size_t> _line_starts;

        size_t clamp(size_t pos) const {
            return std::min(pos, _buffer.size());
        }

        void update_line_starts() {
            _line_starts.clear();
            _line_starts.push_back(0);

            for (auto i = 0; i < _buffer.size(); i++) {
                if (_buffer[i] == '\n') {
                    _line_starts.push_back(i + 1);
                }
            }
        }

        void update_lines_before_erase(size_t start, size_t end) {
            const auto first = std::upper_bound(_line_starts.begin(), _line_starts.end(), start);
            const auto last = std::upper_bound(_line_starts.begin(), _line_starts.end(), end);
            const auto count = end - start;
            auto it = (first != last) ? _line_starts.erase(first, last) : first;

            while (it != _line_starts.end()) {
                *it -= count;
                it += 1;
            }

            if (_line_starts.empty()) {
                _line_starts.push_back(0);
            }
        }

        void update_lines_before_insert(size_t pos, const std::string &text) {
            if (text.empty())
                return;

            auto it = std::upper_bound(_line_starts.begin(), _line_starts.end(), pos);

            for (auto i = 0; i < text.size(); i += 1) {
                if (text[i] == '\n') {
                    it = _line_starts.insert(it, pos + i + 1);
                    it += 1;
                }
            }

            while (it != _line_starts.end()) {
                *it += text.size();
                it += 1;
            }
        }

        std::string get_selection() const {
            if (_selection.empty())
                return "";

            std::string result;
            result.reserve(_selection.size());

            for (size_t i = _selection.start(); i < _selection.end(); ++i) {
                result.push_back(_buffer[i]);
            }
            return result;
        }

        bool delete_selection() {
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
        void insert(const std::string &text) {
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

        void insert(char c) {
            insert(std::string(1, c));
        }

        void erase_before() {
            if (!delete_selection() && _buffer.cursor() > 0) {
                update_lines_before_erase(_buffer.cursor() - 1, _buffer.cursor());
                _recorder.push_undo({EditType::Delete,
                               _buffer.cursor(),
                               true,
                               std::string(1, _buffer[_buffer.cursor() - 1])});
                _buffer.erase_before(1);
            }

            _selection.collapse(_buffer.cursor());
        }

        void erase_after() {
            if (!delete_selection() && _buffer.cursor() < _buffer.size()) {
                update_lines_before_erase(_buffer.cursor(), _buffer.cursor() + 1);
                _recorder.push_undo({EditType::Delete,
                               _buffer.cursor(),
                               false,
                               std::string(1, _buffer[_buffer.cursor()])});
                _buffer.erase_after(1);
            }

            _selection.collapse(_buffer.cursor());
        }

        bool can_undo() {
            return _recorder.can_undo();
        }

        bool can_redo() {
            return _recorder.can_redo();
        }

        void undo() {
            if (_recorder.can_undo()) {
                const auto &o = _recorder.pop_undo();
                _recorder.push_redo(o);

                if (o.type == EditType::Delete) {
                    if (o.backspace) {
                        update_lines_before_insert(o.pos - o.text.size(), o.text);
                        _buffer.set_cursor(o.pos - o.text.size());
                        _buffer.insert(o.text);
                    } else {
                        update_lines_before_insert(o.pos, o.text);
                        _buffer.set_cursor(o.pos);
                        _buffer.insert(o.text);
                        _buffer.set_cursor(o.pos);
                    }
                }

                if (o.type == EditType::Insert) {
                    update_lines_before_erase(o.pos, o.pos + o.text.size());
                    _buffer.set_cursor(o.pos);
                    _buffer.erase_after(o.text.size());
                }

                _recorder.break_merge();
            }

            collapse(_buffer.cursor());
        }

        void redo() {
            if (_recorder.can_redo()) {
                const auto &o = _recorder.pop_redo();
                _recorder.push_undo(o, true);

                if (o.type == EditType::Delete) {
                    if (o.backspace) {
                        update_lines_before_erase(o.pos - o.text.size(), o.pos);
                        _buffer.set_cursor(o.pos);
                        _buffer.erase_before(o.text.size());
                    } else {
                        update_lines_before_erase(o.pos, o.pos + o.text.size());
                        _buffer.set_cursor(o.pos);
                        _buffer.erase_after(o.text.size());
                    }
                }

                if (o.type == EditType::Insert) {
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

        void reset(const std::string &text) {
            std::string t;
            t.reserve(text.size());

            for (auto i = 0; i < text.size(); i++) {
                if (text[i] == '\r') {
                    t.push_back('\n');

                    if (i < text.size() - 1 && text[i + 1] == '\n') {
                        i += 1;
                    }
                } else {
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

        void set_anchor(size_t pos) {
            _selection.anchor = clamp(pos);
            _recorder.break_merge();
        }

        void set_cursor(size_t pos) {
            _selection.cursor = clamp(pos);
            _buffer.set_cursor(pos);
            _recorder.break_merge();
        }

        size_t anchor() const {
            return _selection.anchor;
        }

        size_t cursor() const {
            return _selection.cursor;
        }

        size_t sel_start() const {
            return _selection.start();
        }

        size_t sel_end() const {
            return _selection.end();
        }

        void collapse(size_t pos) {
            _selection.collapse(clamp(pos));
            _buffer.set_cursor(pos);
            _recorder.break_merge();
        }

        void select_all() {
            set_anchor(0);
            set_cursor(_buffer.size());
        }

        size_t line_start(size_t i) const {
            assert(i <= _line_starts.size());
            return i < _line_starts.size() ? _line_starts[i] : _buffer.size();
        }

        size_t line_size(size_t i) const {
            const auto j = line_start(i);
            const auto k = line_start(i + 1);
            return k - j;
        }

        size_t line_count() const {
            return _line_starts.size();
        }

        size_t line_index(size_t pos) const {
            const auto it = std::upper_bound(_line_starts.begin(), _line_starts.end(), pos);
            return it - 1 - _line_starts.begin();
        }

        size_t size() const {
            return _buffer.size();
        }

        std::string to_string() {
            const auto buffer = _buffer.to_buffer();
            return std::string(buffer.begin(), buffer.end());
        }

        char get_char(size_t line, size_t column) {
            const auto i = line_start(line);
            const auto j = line_start(line + 1);
            return (i + column < j) ? _buffer[i + column] : '\0';
        }
    };

}