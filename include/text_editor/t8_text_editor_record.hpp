#pragma once
#include <string>
#include <stack>

namespace t8::text_editor {

    enum class EditType {
        Insert,
        Delete
    };

    struct EditRecord {
        EditType type;
        size_t pos;
        bool backspace;
        std::string text;
    };

    class EditRecorder {
    private:
        std::stack<EditRecord> _undos;
        std::stack<EditRecord> _redos;

        bool _merging = true;

    public:
        bool can_undo() const { return !_undos.empty(); }
        bool can_redo() const { return !_redos.empty(); }

        EditRecord pop_undo() {
            auto e = _undos.top();
            _undos.pop();
            return e;
        }

        EditRecord pop_redo() {
            auto e = _redos.top();
            _redos.pop();
            return e;
        }

        void break_merge() {
            _merging = false;
        }

        void clear_redo() {
            while (!_redos.empty())
                _redos.pop();
        }

        void clear_undo() {
            while (!_undos.empty())
                _undos.pop();
        }

        void push_redo(EditRecord e) {
            _redos.push(std::move(e));
        }

        void push_undo(EditRecord e, bool from_redo = false) {
            if (!from_redo)
                clear_redo();

            if (!_merging || _undos.empty()) {
                _undos.push(std::move(e));
                _merging = true;
                return;
            }

            auto &o = _undos.top();

            if (o.type != e.type) {
                _undos.push(std::move(e));
                return;
            }

            if (o.type == EditType::Insert) {
                if (e.pos == o.text.size() + o.pos) {
                    o.text.append(e.text);
                } else {
                    _undos.push(std::move(e));
                }
            }

            if (o.type == EditType::Delete) {
                if (o.backspace != e.backspace) {
                    _undos.push(std::move(e));
                } else if (e.backspace && e.pos == o.pos - o.text.size()) {
                    o.text.insert(0, e.text);
                } else if (!e.backspace && e.pos == o.pos) {
                    o.text.append(e.text);
                } else {
                    _undos.push(std::move(e));
                }
            }
        }
    };
}