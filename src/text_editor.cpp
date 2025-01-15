#include "text_editor.h"
#include <algorithm>
#include <iostream>
#include <numeric>

namespace t8y {

    void TextEditor::setText(const std::string &text) {
        m_lines = parseText(text);
        sanitize(m_state.ancher);
        sanitize(m_state.cursor);
    }

    std::string TextEditor::getText() {
        return getText(
            Coord(),
            Coord(
                static_cast<int>(m_lines.size()),
                0));
    }

    std::string TextEditor::getText(const Coord &start, const Coord &end, const char &wrap) {
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

    std::string TextEditor::getSelection() {
        Coord start = std::min(m_state.ancher, m_state.cursor);
        Coord end = std::max(m_state.ancher, m_state.cursor);
        return getText(start, end);
    }

    void TextEditor::insert(const std::string &text) {
        UndoRecord rec;
        Coord start = std::min(m_state.ancher, m_state.cursor);
        Coord end = std::max(m_state.ancher, m_state.cursor);

        rec.overwritten = getSelection();
        rec.content = text;
        rec.type = UndoRecord::Type::Add;
        rec.before = m_state;

        removeRange(start, end);

        Coord next = insertAt(start, text);
        rec.start = start;
        rec.end = next;

        setCursor(next);
        setAncher(next);

        m_undos.push(rec);
        m_redos.clear();
    }

    void TextEditor::backspace() {
        if (isSelected())
            cut();
        else {
            UndoRecord rec;
            Coord pos = m_state.cursor;

            if (pos.column == 0) {
                if (pos.line == 0)
                    return;

                auto &prevline = m_lines[pos.line - 1];
                auto &line = m_lines[pos.line];
                Coord next = Coord(pos.line - 1, static_cast<int>(prevline.size()));

                prevline.insert(prevline.end(), line.begin(), line.end());

                rec.type = UndoRecord::Type::Remove;
                rec.content = '\n';
                rec.before = m_state;
                rec.start = next;
                rec.end = pos;
                m_undos.push(rec);
                m_redos.clear();

                removeIndex(m_lines, pos.line, pos.line + 1);

                setCursor(next);
                setAncher(next);
            } else {
                auto &line = m_lines[pos.line];
                Coord next = Coord(pos.line, pos.column - 1);

                rec.type = UndoRecord::Type::Remove;
                rec.content = line[pos.column - 1].first;
                rec.before = m_state;
                rec.start = next;
                rec.end = pos;
                m_undos.push(rec);
                m_redos.clear();

                removeIndex(line, pos.column - 1, pos.column);

                setCursor(next);
                setAncher(next);
            }
        }
    }

    void TextEditor::cut() {
        UndoRecord rec;
        Coord pos = std::min(m_state.cursor, m_state.ancher);

        rec.type = UndoRecord::Type::Remove;
        rec.before = m_state;
        rec.content = getSelection();
        rec.start = std::min(m_state.cursor, m_state.ancher);
        rec.end = std::max(m_state.cursor, m_state.ancher);

        m_undos.push(rec);
        m_redos.clear();

        removeRange(m_state.cursor, m_state.ancher);

        if (m_lines.empty())
            m_lines.push_back(Line());

        setAncher(pos);
        setCursor(pos);
    }

    void TextEditor::undo() {
        if (!undoable())
            return;

        auto record = m_undos.top();
        m_undos.pop();
        m_redos.push(record);

        if (record.type == UndoRecord::Type::Add) {
            removeRange(record.start, record.end);
            insertAt(record.start, record.overwritten);
            setState(record.before);
        }

        if (record.type == UndoRecord::Type::Remove) {
            insertAt(record.start, record.content);
            setState(record.before);
        }
    }

    void TextEditor::redo() {
        if (!redoable())
            return;

        auto record = m_redos.top();
        m_redos.pop();
        m_undos.push(record);

        if (record.type == UndoRecord::Type::Add) {
            removeRange(record.before.ancher, record.before.cursor);
            insertAt(record.start, record.content);
            setAncher(record.end);
            setCursor(record.end);
        }

        if (record.type == UndoRecord::Type::Remove) {
            removeRange(record.start, record.end);
            setAncher(record.start);
            setCursor(record.start);
        }
    }

    bool TextEditor::undoable() {
        return !m_undos.empty();
    }

    bool TextEditor::redoable() {
        return !m_redos.empty();
    }

    size_t TextEditor::lineCount() {
        return m_lines.size();
    }

    size_t TextEditor::totalCount() {
        return std::accumulate(
                   m_lines.begin(),
                   m_lines.end(), 0,
                   [](size_t prev, auto next) {
                       return prev + next.size() + 1;
                   }) -
               1UL;
    }

    void TextEditor::selectAll() {
        setAncher({0, 0});
        setCursor({static_cast<int>(m_lines.size()), static_cast<int>(m_lines.back().size())});
    }

    void TextEditor::move(const MoveType &type, bool both) {
        auto pos = m_state.cursor;
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

    bool TextEditor::isSelected() {
        return m_state.ancher != m_state.cursor;
    }

    void TextEditor::setAncher(const Coord &ancher) {
        m_state.ancher = ancher;
        sanitize(m_state.ancher);
    }

    void TextEditor::setCursor(const Coord &cursor) {
        m_state.cursor = cursor;
        sanitize(m_state.cursor);
    }

    void TextEditor::setState(const EditorState &state) {
        m_state = state;
        sanitize(m_state.ancher);
        sanitize(m_state.cursor);
    }

    const Coord &TextEditor::ancher() const {
        return m_state.ancher;
    }

    const Coord &TextEditor::cursor() const {
        return m_state.cursor;
    }

    Coord TextEditor::insertAt(const Coord &at, const std::string &text) {
        auto inserts = parseText(text);
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
        removeIndex(m_lines, at.line, at.line + 1);
        m_lines.insert(std::next(m_lines.begin(), at.line), inserts.begin(), inserts.end());

        return next;
    }

    void TextEditor::sanitize(Coord &coord) const {
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

    void TextEditor::advance(Coord &coord) const {
        if (coord.line < static_cast<int>(m_lines.size())) {
            const Line &line = m_lines[coord.line];

            if (coord.column + 1 <= static_cast<int>(line.size())) {
                coord = {coord.line, coord.column + 1};
            } else {
                coord = {coord.line + 1, 0};
            }
        }
    }

    void TextEditor::eachLine(int from, int to, EachLineFn fn) {
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

    Lines TextEditor::parseText(const std::string &text) {
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

    void TextEditor::removeRange(const Coord &from, const Coord &to) {
        auto l = std::min(from, to);
        auto r = std::max(from, to);

        if (l == r)
            return;

        if (l.line == r.line) {
            removeIndex(m_lines[l.line], l.column, r.column);
        } else {
            auto &first = m_lines[l.line];
            auto &last = m_lines[r.line];

            removeIndex(first, l.column, first.size());
            removeIndex(last, 0, r.column);

            first.insert(first.end(), last.begin(), last.end());

            removeIndex(m_lines, l.line + 1, r.line + 1);
        }
    }
}