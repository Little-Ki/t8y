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

        size_t clamp(size_t pos) const {
            return std::min(pos, buf.size());
        }

        std::string get_selection() const {
            if (sel.empty())
                return "";

            std::string result;
            result.reserve(sel.size());
            for (size_t i = 0; i < sel.size(); ++i) {
                result.push_back(buf[sel.start() + i]);
            }
            return result;
        }

        bool delete_selection() {
            if (sel.empty())
                return false;

            rec.break_merge();
            rec.push_undo({EditType::Delete,
                           sel.cursor,
                           sel.cursor >= sel.anchor,
                           get_selection()});
            rec.break_merge();

            buf.set_cursor(sel.end());
            buf.erase_before(sel.size());
            sel.collapse(sel.start());
            return true;
        }

    public:
        void insert(const std::string &text) {
            delete_selection();

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
            if (delete_selection()) {
                return;
            }

            if (buf.cursor() == 0) {
                return;
            }

            rec.push_undo({EditType::Delete,
                           buf.cursor(),
                           true,
                           std::string(1, buf[buf.cursor() - 1])});
            buf.erase_before(1);
            sel.collapse(buf.cursor());
        }

        void erase_after() {
            if (delete_selection()) {
                return;
            }

            if (buf.cursor() == buf.size()) {
                return;
            }

            rec.push_undo({EditType::Delete,
                           buf.cursor(),
                           false,
                           std::string(1, buf[buf.cursor()])});
            buf.erase_after(1);
            sel.collapse(buf.cursor());
        }

        bool can_undo() {
            return rec.can_undo();
        }

        bool can_redo() {
            return rec.can_redo();
        }

        void undo() {
            if (!rec.can_undo()) {
                return;
            }

            const auto &o = rec.pop_undo();
            rec.push_redo(o);

            if (o.type == EditType::Delete) {
                if (o.backspace) {
                    buf.set_cursor(o.pos - o.text.size());
                    buf.insert(o.text);
                } else {
                    buf.set_cursor(o.pos);
                    buf.insert(o.text);
                }
            }

            if (o.type == EditType::Insert) {
                buf.set_cursor(o.pos);
                buf.erase_after(o.text.size());
            }
            sel.collapse(buf.cursor());
        }

        void redo() {
            if (!rec.can_redo()) {
                return;
            }

            const auto &o = rec.pop_redo();
            rec.push_undo(o, true);

            if (o.type == EditType::Delete) {
                if (o.backspace) {
                    buf.set_cursor(o.pos);
                    buf.erase_before(o.text.size());
                } else {
                    buf.set_cursor(o.pos);
                    buf.erase_after(o.text.size());
                }
            }

            if (o.type == EditType::Insert) {
                buf.set_cursor(o.pos);
                buf.insert(o.text);
            }
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
            sel.collapse(0);
            rec.clear_redo();
            rec.clear_undo();
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

        size_t line_start(size_t i) const {
            return buf.line_start(i);
        }

        size_t line_size(size_t i) const {
            return buf.line_size(i);
        }

        size_t line_count() const {
            return buf.line_count();
        }

        std::string to_string() {
            const auto buffer = buf.to_buffer();
            return std::string(buffer.begin(), buffer.end());
        }

        char get_char(size_t line, size_t column) {
            const auto i = buf.line_start(line);
            const auto j = buf.line_start(line + 1);
            return (i + column < j) ? buf[i + column] : '\0';
        }
    };

}