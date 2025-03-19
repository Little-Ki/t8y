#include "t8_textedit.h"
#include <algorithm>
#include <iostream>
#include <numeric>

namespace t8 {

    void TextEdit::setText(const std::string &text) {
        m_lines = parse(text);
        sanitize(_state.ancher);
        sanitize(_state.cursor);
    }

    std::string TextEdit::getText() {
        return getText(
            Coord(),
            Coord(
                static_cast<int>(m_lines.size()),
                0));
    }

    std::string TextEdit::getText(const Coord &start, const Coord &end, const char &wrap) {
        assert(start <= end);
        std::string result;

        if (start == end)
            return result;

        auto prevLine = start.line;
        for (auto it = start; it < end; advance(it)) {
            if (prevLine != it.line)
                result += wrap;

            const Line &line = m_lines[it.line];
            prevLine = it.line;

            if (!line.empty() && it.column < static_cast<int>(line.size())) {
                result += line[it.column].first;
            }
        }

        return result;
    }

    std::string TextEdit::getSelection() {
        Coord start = std::min(_state.ancher, _state.cursor);
        Coord end = std::max(_state.ancher, _state.cursor);
        return getText(start, end);
    }

    void TextEdit::insert(const std::string &text) {
        UndoRecord rec;
        Coord start = std::min(_state.ancher, _state.cursor);
        Coord end = std::max(_state.ancher, _state.cursor);

        rec.overwritten = getSelection();
        rec.content = text;
        rec.type = UndoRecord::Type::Add;
        rec.before = _state;

        erase(start, end);

        Coord next = insertAt(start, text);
        rec.start = start;
        rec.end = next;

        setCursor(next);
        setAncher(next);

        _undos.push(rec);
        _redos.clear();
    }

    void TextEdit::backspace() {
        if (isSelected())
            cut();
        else {
            UndoRecord rec;
            Coord pos = _state.cursor;

            if (pos.column == 0) {
                if (pos.line == 0)
                    return;

                auto &prevline = m_lines[pos.line - 1];
                auto &line = m_lines[pos.line];
                Coord next = Coord(pos.line - 1, static_cast<int>(prevline.size()));

                prevline.insert(prevline.end(), line.begin(), line.end());

                rec.type = UndoRecord::Type::Remove;
                rec.content = '\n';
                rec.before = _state;
                rec.start = next;
                rec.end = pos;
                _undos.push(rec);
                _redos.clear();

                erase(m_lines, pos.line, pos.line + 1);

                setCursor(next);
                setAncher(next);
            } else {
                auto &line = m_lines[pos.line];
                Coord next = Coord(pos.line, pos.column - 1);

                rec.type = UndoRecord::Type::Remove;
                rec.content = line[pos.column - 1].first;
                rec.before = _state;
                rec.start = next;
                rec.end = pos;
                _undos.push(rec);
                _redos.clear();

                erase(line, pos.column - 1, pos.column);

                setCursor(next);
                setAncher(next);
            }
        }
    }

    void TextEdit::cut() {
        UndoRecord rec;
        Coord pos = std::min(_state.cursor, _state.ancher);

        rec.type = UndoRecord::Type::Remove;
        rec.before = _state;
        rec.content = getSelection();
        rec.start = std::min(_state.cursor, _state.ancher);
        rec.end = std::max(_state.cursor, _state.ancher);

        _undos.push(rec);
        _redos.clear();

        erase(_state.cursor, _state.ancher);

        if (m_lines.empty())
            m_lines.push_back(Line());

        setAncher(pos);
        setCursor(pos);
    }

    void TextEdit::undo() {
        if (!undoable())
            return;

        auto record = _undos.top();
        _undos.pop();
        _redos.push(record);

        if (record.type == UndoRecord::Type::Add) {
            erase(record.start, record.end);
            insertAt(record.start, record.overwritten);
            setState(record.before);
        }

        if (record.type == UndoRecord::Type::Remove) {
            insertAt(record.start, record.content);
            setState(record.before);
        }
    }

    void TextEdit::redo() {
        if (!redoable())
            return;

        auto record = _redos.top();
        _redos.pop();
        _undos.push(record);

        if (record.type == UndoRecord::Type::Add) {
            erase(record.before.ancher, record.before.cursor);
            insertAt(record.start, record.content);
            setAncher(record.end);
            setCursor(record.end);
        }

        if (record.type == UndoRecord::Type::Remove) {
            erase(record.start, record.end);
            setAncher(record.start);
            setCursor(record.start);
        }
    }

    bool TextEdit::undoable() {
        return !_undos.empty();
    }

    bool TextEdit::redoable() {
        return !_redos.empty();
    }

    size_t TextEdit::lineCount() {
        return m_lines.size();
    }

    size_t TextEdit::totalCount() {
        return std::accumulate(
                   m_lines.begin(),
                   m_lines.end(), 0,
                   [](size_t prev, auto next) {
                       return prev + next.size() + 1;
                   }) -
               1UL;
    }

    void TextEdit::selectAll() {
        setAncher({0, 0});
        setCursor({static_cast<int>(m_lines.size()), static_cast<int>(m_lines.back().size())});
    }

    void TextEdit::move(const MoveType &type, bool both) {
        auto pos = _state.cursor;
        sanitize(pos);

        if (type == MoveType::PrevColumn) {
            if (pos.column == 0) {
                if (pos.line == 0)
                    return;
                pos.column = static_cast<int>(m_lines[pos.line - 1].size());
                pos.line -= 1;
            } else {
                pos.column -= 1;
            }
        }

        if (type == MoveType::NextColumn) {
            const auto &line = m_lines[pos.line];
            if (pos.column == line.size()) {
                if (pos.line == m_lines.size())
                    return;
                pos.line += 1;
                pos.column = 0;
            } else {
                pos.column += 1;
            }
        }

        if (type == MoveType::PrevLine) {
            if (pos.line == 0)
                return;
            pos.column = std::min(pos.column, static_cast<int>(m_lines[pos.line - 1].size()));
            pos.line -= 1;
        }

        if (type == MoveType::NextLine) {
            if (pos.line == m_lines.size())
                return;
            pos.column = std::min(pos.column, static_cast<int>(m_lines[pos.line + 1].size()));
            pos.line += 1;
        }

        setCursor(pos);
        if (both)
            setAncher(pos);
    }

    bool TextEdit::isSelected() {
        return _state.ancher != _state.cursor;
    }

    void TextEdit::setAncher(const Coord &ancher) {
        _state.ancher = ancher;
        sanitize(_state.ancher);
    }

    void TextEdit::setCursor(const Coord &cursor) {
        _state.cursor = cursor;
        sanitize(_state.cursor);
    }

    void TextEdit::setState(const EditorSelection &state) {
        _state = state;
        sanitize(_state.ancher);
        sanitize(_state.cursor);
    }

    const Coord &TextEdit::ancher() const {
        return _state.ancher;
    }

    const Coord &TextEdit::cursor() const {
        return _state.cursor;
    }

    Coord TextEdit::insertAt(const Coord &at, const std::string &text) {
        auto inserts = parse(text);
        auto &first = inserts.front();
        auto &last = inserts.back();
        auto next = inserts.size() > 1 ? Coord(
                                             at.line + inserts.size() - 1,
                                             last.size())
                                       : Coord(
                                             at.line,
                                             at.column + text.size());
        auto &line = m_lines[at.line];

        first.insert(first.begin(), line.begin(), std::next(line.begin(), at.column));
        last.insert(last.end(), std::next(line.begin(), at.column), line.end());
        erase(m_lines, at.line, at.line + 1);
        m_lines.insert(std::next(m_lines.begin(), at.line), inserts.begin(), inserts.end());

        return next;
    }

    void TextEdit::sanitize(Coord &coord) const {
        int line = std::max(0, std::min(static_cast<int>(m_lines.size() - 1), coord.line));
        int column = 0;
        if (!m_lines.empty()) {
            if (line < coord.line)
                column = static_cast<int>(m_lines[line].size());
            else
                column = std::clamp(coord.column, 0, static_cast<int>(m_lines[line].size()));
        }

        coord = {line, column};
    }

    void TextEdit::advance(Coord &coord) const {
        if (coord.line < static_cast<int>(m_lines.size())) {
            const Line &line = m_lines[coord.line];

            if (coord.column + 1 <= static_cast<int>(line.size())) {
                coord = {coord.line, coord.column + 1};
            } else {
                coord = {coord.line + 1, 0};
            }
        }
    }

    void TextEdit::eachLine(int from, int to, EachLineFn fn) {
        from = std::clamp(from, 0, static_cast<int>(m_lines.size()));
        to = std::clamp(to, 0, static_cast<int>(m_lines.size()));

        if (from > to)
            std::swap(from, to);

        if (m_lines.empty())
            return;

        for (auto ln = from; ln < to; ln++) {
            fn(m_lines[ln], ln);
        }
    }

    Lines TextEdit::parse(const std::string &text) {
        Lines result;
        result.push_back(Line());

        for (auto it = text.begin(); it != text.end(); it++) {
            const auto ch = *it;

            if (ch == '\n') {
                result.push_back(Line());
            } else if (ch != '\r') {
                result.back().push_back(std::make_pair(ch, 0));
            }
        }

        return result;
    }

    void TextEdit::erase(const Coord &from, const Coord &to) {
        auto l = std::min(from, to);
        auto r = std::max(from, to);

        if (l == r)
            return;

        if (l.line == r.line) {
            erase(m_lines[l.line], l.column, r.column);
        } else {
            auto &first = m_lines[l.line];
            auto &last = m_lines[r.line];

            erase(first, l.column, first.size());
            erase(last, 0, r.column);

            first.insert(first.end(), last.begin(), last.end());

            erase(m_lines, l.line + 1, r.line + 1);
        }
    }
}