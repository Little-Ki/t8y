#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <stack>
#include <string>
#include <vector>

namespace t8y {

    template <typename T, size_t SIZE>
    class SizedStack : public std::vector<T> {
    public:
        SizedStack() = default;

        void push(const T &val) {
            if (this->size() + 1 == SIZE)
                this->erase(this->begin());
            this->push_back(val);
        }

        void push(T &&val) {
            if (this->size() + 1 == SIZE)
                this->erase(this->begin());
            this->push_back(val);
        }

        T &top() {
            return this->back();
        }

        void pop() {
            this->pop_back();
        }

        bool empty() {
            return this->size() == 0;
        }
        
    };

    class Coord {
    public:
        int line = 0, column = 0;

        Coord() = default;

        Coord(int ln, int col) : line(ln), column(col) {
            assert(line >= 0 && col >= 0);
        }

        static Coord invalid() {
            static Coord invalid(-1, -1);
            return invalid;
        }

        bool operator==(const Coord &o) const {
            return line == o.line && column == o.column;
        }

        bool operator!=(const Coord &o) const {
            return line != o.line || column != o.column;
        }

        bool operator<(const Coord &o) const {
            if (line != o.line)
                return line < o.line;
            return column < o.column;
        }

        bool operator>(const Coord &o) const {
            if (line != o.line)
                return line > o.line;
            return column > o.column;
        }

        bool operator<=(const Coord &o) const {
            if (line != o.line)
                return line < o.line;
            return column <= o.column;
        }

        bool operator>=(const Coord &o) const {
            if (line != o.line)
                return line > o.line;
            return column >= o.column;
        }
    };

    struct EditorState {
        Coord ancher;
        Coord cursor;
    };

    using Character = std::pair<char, uint8_t>;
    using Line = std::vector<Character>;
    using Lines = std::vector<Line>;

    class UndoRecord {
    public:
        enum class Type {
            Add,
            Remove
        };

        Type type;

        std::string overwritten;
        std::string content;

        Coord start;
        Coord end;

        EditorState before;
    };

    using EachLineFn = std::function<void(Line &, const int &)>;

    enum MoveType {
        PrevColumn,
        NextColumn,
        PrevLine,
        NextLine
    };

    class TextEditor {
    public:
        TextEditor() : m_lines({Line()}) {};

        void setText(const std::string &text);

        std::string getText();

        std::string getText(const Coord &begin, const Coord &end, const char &wrap = '\n');

        std::string getSelection();

    public:
        void insert(const std::string &text);

        void backspace();

        void cut();

        void undo();

        void redo();

        bool undoable();

        bool redoable();

        size_t lineCount();

        size_t totalCount();

        void selectAll();

        void move(const MoveType &type, bool both);

    public:
        bool isSelected();

        const Coord &ancher() const;

        void setAncher(const Coord &ancher);

        const Coord &cursor() const;

        void setCursor(const Coord &cursor);

        void setState(const EditorState &state);

        void eachLine(int from, int to, EachLineFn fn);

    private:
        EditorState m_state;
        Lines m_lines;

        SizedStack<UndoRecord, 64> m_undos;
        SizedStack<UndoRecord, 64> m_redos;

    private:
        Coord insertAt(const Coord &at, const std::string &text);

        void sanitize(Coord &coord) const;

        void advance(Coord &coord) const;

        Lines parseText(const std::string &text);

        void removeRange(const Coord &from, const Coord &to);

        template <typename T>
        void removeIndex(std::vector<T> &vec, int a, int b) {
            a = std::clamp(a, 0, static_cast<int>(vec.size()));
            b = std::clamp(b, 0, static_cast<int>(vec.size()));

            if (a > b)
                std::swap(a, b);

            if (vec.empty())
                return;

            vec.erase(
                std::next(vec.begin(), a),
                std::next(vec.begin(), b));
        }
    };

}