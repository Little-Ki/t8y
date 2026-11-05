#pragma once

#include <stack>
#include <string>

namespace t8::ted
{

    enum class RecordType
    {
        Insert,
        Delete
    };

    struct Record
    {
        RecordType type;
        size_t pos;
        bool backspace;
        std::string text;
    };

    class Recorder
    {
    private:
        std::stack<Record> _undos;
        std::stack<Record> _redos;

        bool _merging = true;

    public:
        bool can_undo() const { return !_undos.empty(); }
        bool can_redo() const { return !_redos.empty(); }

        Record pop_undo()
        {
            auto e = _undos.top();
            _undos.pop();
            return e;
        }

        Record pop_redo()
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

        void push_redo(Record e)
        {
            _redos.push(std::move(e));
        }

        void push_undo(Record e, bool from_redo = false)
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

            if (o.type == RecordType::Insert)
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

            if (o.type == RecordType::Delete)
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

}