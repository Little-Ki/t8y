#include "t8_scene_editor_script.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keybd.h"
#include "t8_input_mouse.h"
#include "t8_utils_timer.h"

#include "t8_text_editor.hpp"

using namespace t8::core;
using namespace t8::utils;
using namespace t8::input;

namespace t8::scene
{
    struct EditorState
    {
        int scroll_x = 0;
        int scroll_y = 0;
        bool dragging = false;
        t8::text_editor::TextEditor editor;
    };

    static EditorState state;

    size_t editor_script_edit_visual_column(size_t line, size_t cursor)
    {
        if (line < 0 || line >= state.editor.line_count())
            return 0;

        const auto line_start = state.editor.line_start(line);
        const auto line_size = state.editor.line_size(line);
        const auto max_column = std::min(line_size, cursor - line_start);
        size_t result = 0;

        for (size_t col = 0; col < max_column; col += 1)
        {
            const auto ch = state.editor.get_char(line, col);
            const auto w = ch == '\t' ? 8 : 4;
            result += w;
        }

        return result;
    }

    void editor_script_edit_cursor_lifting(size_t next_line, size_t visual_column, bool collapse = true)
    {
        if (next_line < 0 || next_line >= state.editor.line_count())
            return;

        const auto line_start = state.editor.line_start(next_line);
        const auto line_size = state.editor.line_size(next_line);
        size_t temp = 0;
        size_t col = 0;

        for (; col < line_size;)
        {
            const auto ch = state.editor.get_char(next_line, col);
            const auto w = ch == '\t' ? 8 : 4;
            if (temp >= visual_column)
            {
                break;
            }
            temp += w;
            col += 1;
        }

        const size_t max_col = next_line == state.editor.line_count() - 1 ? line_size : line_size - 1;
        col = std::min(max_col, col);

        if (collapse)
            state.editor.collapse(line_start + col);
        else
            state.editor.set_cursor(line_start + col);
    }

    void update_script_editor()
    {
        auto auto_scroll = true;
        auto cursor = state.editor.cursor();

        if (!ctx_inputs().empty())
        {
            const auto text = ctx_inputs().front();
            ctx_inputs().pop();

            for (const auto &ch : text)
                if (ch & 0x80)
                    return;

            state.editor.insert(text);
        }
        else
        {
            if (keybd_triggered(SCANCODE_ENTER) ||
                keybd_triggered(SCANCODE_RETURN))
            {
                state.editor.insert('\n');
            }
            else if (keybd_triggered(SCANCODE_BACKSPACE))
            {
                state.editor.erase_before();
            }
            else if (keybd_triggered(SCANCODE_DELETE))
            {
                state.editor.erase_after();
            }
            else if (keybd_triggered(SCANCODE_UP))
            {
                const auto cursor = state.editor.cursor();
                const auto line = state.editor.line_index(cursor);
                if (line > 0)
                {
                    const auto col = editor_script_edit_visual_column(line, cursor);
                    editor_script_edit_cursor_lifting(line - 1, col);
                }
            }
            else if (keybd_triggered(SCANCODE_DOWN))
            {
                const auto cursor = state.editor.cursor();
                const auto line = state.editor.line_index(cursor);
                if (line < state.editor.line_count() - 1)
                {
                    const auto col = editor_script_edit_visual_column(line, cursor);
                    editor_script_edit_cursor_lifting(line + 1, col);
                }
            }
            else if (keybd_triggered(SCANCODE_LEFT))
            {
                if (cursor > 0)
                {
                    state.editor.collapse(cursor - 1);
                }
            }
            else if (keybd_triggered(SCANCODE_RIGHT))
            {
                state.editor.collapse(cursor + 1);
            }
            else if (keybd_pressed(SCANCODE_A))
            {
                if (!keybd_alt() && !keybd_shift() && keybd_ctrl())
                {
                    state.editor.select_all();
                }
            }
            else if (keybd_pressed(SCANCODE_Z))
            {
                if (!keybd_alt() && !keybd_shift() && keybd_ctrl())
                {
                    state.editor.undo();
                }
            }
            else if (keybd_pressed(SCANCODE_Y))
            {
                if (!keybd_alt() && !keybd_shift() && keybd_ctrl())
                {
                    state.editor.redo();
                }
            }
            else if (keybd_pressed(SCANCODE_TAB))
            {
                if (!keybd_alt() && !keybd_shift() && !keybd_ctrl())
                {
                    state.editor.insert("\t");
                }
            }
            else if (mouse_inside(8, 11, 120, 128) && mouse_z() != 0)
            {
                auto min_y = 0;
                auto max_y = std::max(0, static_cast<int>(state.editor.line_count()) - 14);

                if (mouse_z() < 0)
                {
                    state.scroll_y = std::clamp(state.scroll_y + 1, min_y, max_y);
                }
                else
                {
                    state.scroll_y = std::clamp(state.scroll_y - 1, min_y, max_y);
                }
                auto_scroll = false;
            }
            else
            {
                auto_scroll = false;
            }
        }

        if (mouse_inside(0, 11, 128, 128) || state.dragging)
        {
            const auto local_x = mouse_x() - 8 + state.scroll_x * 4;
            const auto local_y = mouse_y() - 11 + state.scroll_y * 8;

            auto line = local_y / 8;

            if (line >= static_cast<int>(state.editor.line_count()))
                line = static_cast<int>(state.editor.line_count()) - 1;

            if (line < 0)
                line = 0;

            const auto line_start = state.editor.line_start(line);
            const auto line_size = state.editor.line_size(line);
            const auto max_size = line == state.editor.line_count() - 1 ? line_size : line_size - 1;

            auto cursor_index = line_start;
            int x = 0;
            int p = 0;

            for (size_t col = 0; col < max_size; col += 1)
            {
                const auto ch = state.editor.get_char(line, col);
                const auto w = ch == '\t' ? 8 : 4;
                int middle = x + w / 2;

                if (local_x >= p && local_x < middle)
                {
                    cursor_index = line_start + col;
                }

                p = middle;
                x += w;
            }

            if (local_x >= p)
            {
                cursor_index = line_start + max_size;
            }

            if (mouse_pressed(1))
            {
                state.editor.collapse(cursor_index);
                state.dragging = true;
                auto_scroll = true;
            }
            else if (mouse_down(1))
            {
                state.editor.set_cursor(cursor_index);
                auto_scroll = true;
            }
            else
            {
                state.dragging = false;
            }
        }

        if (auto_scroll)
        {
            const auto cursor = state.editor.cursor();
            const auto line = state.editor.line_index(cursor);
            const auto line_start = state.editor.line_start(line);
            const auto line_size = state.editor.line_size(line);
            const auto column = cursor - line_start;
            const auto delta_y = static_cast<int>(line) - state.scroll_y;
            const auto delta_x = static_cast<int>(column) - state.scroll_x;

            if (delta_y > 12)
            {
                state.scroll_y = static_cast<int>(line) - 13;
            }
            else if (delta_y < 1)
            {
                state.scroll_y = static_cast<int>(line);
            }

            if (delta_x > 27)
            {
                state.scroll_x = static_cast<int>(column) - 28;
            }
            else if (delta_x < 1)
            {
                state.scroll_x = static_cast<int>(column);
            }
        }
    }

    void draw_script_editor()
    {
        painter_rect(0, 11, 8, 117, 14, true);
        painter_rect(8, 11, 120, 117, 15, true);

        const auto sel_start = state.editor.sel_start();
        const auto sel_end = state.editor.sel_end();
        const auto cursor = state.editor.cursor();

        painter_camera(8, 11);
        painter_clip(8, 11, 120, 117);

        auto line_count = state.editor.line_count();
        auto above = state.scroll_y;
        auto below = std::min(line_count, static_cast<size_t>(above) + 16);

        auto x = -state.scroll_x * 4;
        auto y = -state.scroll_y * 8 + above * 8;

        for (auto line = above; line < below; line += 1)
        {
            auto line_size = state.editor.line_size(line);
            auto line_start = state.editor.line_start(line);

            for (auto col = 0; col < line_size; col += 1)
            {
                const auto ch = state.editor.get_char(line, col);
                const auto current_index = line_start + col;
                if ((timer_ticks() >> 5) % 2 && current_index == cursor)
                    painter_rect(x, y, 1, 8, 3, true);

                if (current_index >= sel_start && current_index < sel_end && ch != '\n')
                    painter_rect(x, y, ch == '\t' ? 8 : 4, 8, 3, true);

                if (ch == '\t')
                {
                    x += 8;
                }
                else if (ch != '\n')
                {
                    painter_char(ch, x, y, 1);
                    x += 4;
                }
            }

            if ((timer_ticks() >> 5) % 2 && line == below - 1 && line_start + line_size == cursor)
                painter_rect(x, y, 1, 8, 3, true);

            x = -state.scroll_x * 4;
            y += 8;
        }

        painter_camera();
        painter_clip();
    }

    void script_editor_set_script(const std::string &script)
    {
        state.editor.reset(script);
    }

    std::string script_editor_get_script()
    {
        return state.editor.to_string();
    }

}