#pragma once
#include "t8_text_editor_buffer.hpp"
#include "t8_text_editor_record.hpp"
#include "t8_text_editor_selection.hpp"
#include <cmath>

namespace t8::text_editor {

    class TextEditor {
    private:
        GapBuffer buf;
        EditSelection sel;
        EditRecorder rec;
        std::vector<size_t> line_starts;

        size_t clamp(size_t pos) const {
            return std::min(pos, buf.size());
        }

        void update_line_starts() {
            line_starts.clear();
            line_starts.push_back(0);

            for (auto i = 0; i < buf.size(); i++) {
                if (buf[i] == '\n') {
                    line_starts.push_back(i + 1);
                }
            }
        }

        void update_lines_before_erase(size_t start, size_t end) {
            const auto first = std::upper_bound(line_starts.begin(), line_starts.end(), start);
            const auto last = std::upper_bound(line_starts.begin(), line_starts.end(), end);
            const auto count = end - start;
            auto it = (first != last) ? line_starts.erase(first, last) : first;

            while (it != line_starts.end()) {
                *it -= count;
                it += 1;
            }

            if (line_starts.empty()) {
                line_starts.push_back(0);
            }
        }

        void update_lines_before_insert(size_t pos, const std::string &text) {
            if (text.empty())
                return;

            auto it = std::upper_bound(line_starts.begin(), line_starts.end(), pos);

            for (auto i = 0; i < text.size(); i += 1) {
                if (text[i] == '\n') {
                    it = line_starts.insert(it, pos + i + 1);
                    it += 1;
                }
            }

            while (it != line_starts.end()) {
                *it += text.size();
                it += 1;
            }
        }

        std::string get_selection() const {
            if (sel.empty())
                return "";

            std::string result;
            result.reserve(sel.size());

            for (size_t i = sel.start(); i < sel.end(); ++i) {
                result.push_back(buf[i]);
            }
            return result;
        }

        bool delete_selection() {
            if (sel.empty())
                return false;

            rec.break_merge();

            update_lines_before_erase(sel.start(), sel.end());
            rec.push_undo({EditType::Delete,
                           sel.cursor,
                           sel.cursor >= sel.anchor,
                           get_selection()});
            rec.break_merge();

            buf.set_cursor(sel.end());
            buf.erase_before(sel.size());

            sel.collapse(buf.cursor());

            return true;
        }

    public:
        void insert(const std::string &text) {
            delete_selection();
            const auto cursor = buf.cursor();
            update_lines_before_insert(cursor, text);
            rec.push_undo({EditType::Insert,
                           buf.cursor(),
                           false,
                           text});
            buf.insert(text);
            sel.collapse(buf.cursor());
        }

        void insert(char c) {
            insert(std::string(1, c));
        }

        void erase_before() {
            if (!delete_selection() && buf.cursor() > 0) {
                update_lines_before_erase(buf.cursor() - 1, buf.cursor());
                rec.push_undo({EditType::Delete,
                               buf.cursor(),
                               true,
                               std::string(1, buf[buf.cursor() - 1])});
                buf.erase_before(1);
            }

            sel.collapse(buf.cursor());
        }

        void erase_after() {
            if (!delete_selection() && buf.cursor() < buf.size()) {
                update_lines_before_erase(buf.cursor(), buf.cursor() + 1);
                rec.push_undo({EditType::Delete,
                               buf.cursor(),
                               false,
                               std::string(1, buf[buf.cursor()])});
                buf.erase_after(1);
            }

            sel.collapse(buf.cursor());
        }

        bool can_undo() {
            return rec.can_undo();
        }

        bool can_redo() {
            return rec.can_redo();
        }

        void undo() {
            if (rec.can_undo()) {
                const auto &o = rec.pop_undo();
                rec.push_redo(o);

                if (o.type == EditType::Delete) {
                    if (o.backspace) {
                        update_lines_before_insert(o.pos - o.text.size(), o.text);
                        buf.set_cursor(o.pos - o.text.size());
                        buf.insert(o.text);
                    } else {
                        update_lines_before_insert(o.pos, o.text);
                        buf.set_cursor(o.pos);
                        buf.insert(o.text);
                        buf.set_cursor(o.pos);
                    }
                }

                if (o.type == EditType::Insert) {
                    update_lines_before_erase(o.pos, o.pos + o.text.size());
                    buf.set_cursor(o.pos);
                    buf.erase_after(o.text.size());
                }

                rec.break_merge();
            }

            collapse(buf.cursor());
        }

        void redo() {
            if (rec.can_redo()) {
                const auto &o = rec.pop_redo();
                rec.push_undo(o, true);

                if (o.type == EditType::Delete) {
                    if (o.backspace) {
                        update_lines_before_erase(o.pos - o.text.size(), o.pos);
                        buf.set_cursor(o.pos);
                        buf.erase_before(o.text.size());
                    } else {
                        update_lines_before_erase(o.pos, o.pos + o.text.size());
                        buf.set_cursor(o.pos);
                        buf.erase_after(o.text.size());
                    }
                }

                if (o.type == EditType::Insert) {
                    update_lines_before_insert(o.pos, o.text);
                    buf.set_cursor(o.pos);
                    buf.insert(o.text);
                    buf.set_cursor(o.pos + o.text.size());
                }

                rec.break_merge();
            }

            collapse(buf.cursor());
        }

        const auto &buffer() const { return buf; }

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

            buf.reset(t);
            buf.set_cursor(0);
            sel.collapse(0);
            rec.clear_redo();
            rec.clear_undo();
            rec.break_merge();

            update_line_starts();
        }

        void set_anchor(size_t pos) {
            sel.anchor = clamp(pos);
            rec.break_merge();
        }

        void set_cursor(size_t pos) {
            sel.cursor = clamp(pos);
            buf.set_cursor(pos);
            rec.break_merge();
        }

        size_t anchor() const {
            return sel.anchor;
        }

        size_t cursor() const {
            return sel.cursor;
        }

        size_t sel_start() const {
            return sel.start();
        }

        size_t sel_end() const {
            return sel.end();
        }

        void collapse(size_t pos) {
            sel.collapse(clamp(pos));
            buf.set_cursor(pos);
            rec.break_merge();
        }

        void select_all() {
            set_anchor(0);
            set_cursor(buf.size());
        }

        size_t line_start(size_t i) const {
            assert(i <= line_starts.size());
            return i < line_starts.size() ? line_starts[i] : buf.size();
        }

        size_t line_size(size_t i) const {
            const auto j = line_start(i);
            const auto k = line_start(i + 1);
            return k - j;
        }

        size_t line_count() const {
            return line_starts.size();
        }

        size_t size() const {
            return buf.size();
        }

        std::string to_string() {
            const auto buffer = buf.to_buffer();
            return std::string(buffer.begin(), buffer.end());
        }

        char get_char(size_t line, size_t column) {
            const auto i = line_start(line);
            const auto j = line_start(line + 1);
            return (i + column < j) ? buf[i + column] : '\0';
        }
    };

}