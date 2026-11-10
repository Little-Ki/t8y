#include "editor_script.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_mouse.h"
#include "input_keyboard.h"

#include "constants.h"

using namespace t8::input;

namespace t8::scene::editor::script
{

    static size_t visual_column(EditorState &state, size_t line, size_t cursor)
    {
        if (line < 0 || line >= state.editor.line_count())
            return 0;

        const auto line_start = state.editor.line_start(line);
        const auto line_size = state.editor.line_size(line);
        const auto max_column = std::min(line_size, cursor - line_start);
        size_t result = 0;

        for (size_t col = 0; col < max_column; col += 1)
        {
            const auto ch = state.editor.get_char(line_start + col);
            const auto w = ch == '\t' ? 8 : 4;
            result += w;
        }

        return result;
    }

    static void cursor_lifting(EditorState &state, size_t next_line, size_t visual_column, bool collapse = true)
    {
        if (next_line < 0 || next_line >= state.editor.line_count())
            return;

        const auto line_start = state.editor.line_start(next_line);
        const auto line_size = state.editor.line_size(next_line);
        size_t temp = 0;
        size_t col = 0;

        for (; col < line_size;)
        {
            const auto ch = state.editor.get_char(line_start + col);
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

    void update(AppContext *ctx, EditorState &state)
    {

        auto auto_scroll = true;
        auto cursor = state.editor.cursor();
        auto only_ctrl = !keyboard::alt(ctx->keyboard) && !keyboard::shift(ctx->keyboard) && keyboard::ctrl(ctx->keyboard);
        auto without_ctrls = !keyboard::alt(ctx->keyboard) && !keyboard::shift(ctx->keyboard) && !keyboard::ctrl(ctx->keyboard);
        auto is_shift = keyboard::down(ctx->keyboard, SCANCODE_LSHIFT) || keyboard::down(ctx->keyboard, SCANCODE_RSHIFT);

        if (!ctx->inputs.empty())
        {
            const auto text = ctx->inputs.front();
            ctx->inputs.pop();

            for (const auto &ch : text)
                if (ch & 0x80)
                    return;

            state.editor.insert(text);
        }
        else
        {
            if (keyboard::triggered(ctx->keyboard, SCANCODE_ENTER) ||
                keyboard::triggered(ctx->keyboard, SCANCODE_RETURN))
            {
                state.editor.insert('\n');
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_BACKSPACE))
            {
                state.editor.erase_before();
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_DELETE))
            {
                state.editor.erase_after();
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_UP))
            {
                const auto cursor = state.editor.cursor();
                const auto line = state.editor.line_index(cursor);
                if (line > 0)
                {
                    const auto col = visual_column(state, line, cursor);
                    cursor_lifting(state, line - 1, col, !is_shift);
                }
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_DOWN))
            {
                const auto cursor = state.editor.cursor();
                const auto line = state.editor.line_index(cursor);
                if (line < state.editor.line_count() - 1)
                {
                    const auto col = visual_column(state, line, cursor);
                    cursor_lifting(state, line + 1, col, !is_shift);
                }
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_LEFT))
            {
                if (cursor > 0)
                {
                    if (is_shift)
                        state.editor.set_cursor(cursor - 1);
                    else
                        state.editor.collapse(cursor - 1);
                }
            }
            else if (keyboard::triggered(ctx->keyboard, SCANCODE_RIGHT))
            {
                if (is_shift)
                    state.editor.set_cursor(cursor + 1);
                else
                    state.editor.collapse(cursor + 1);
            }
            else if (keyboard::pressed(ctx->keyboard, SCANCODE_A) && only_ctrl)
            {
                state.editor.select_all();
            }
            else if (keyboard::pressed(ctx->keyboard, SCANCODE_Z) && only_ctrl)
            {
                state.editor.undo();
            }
            else if (keyboard::pressed(ctx->keyboard, SCANCODE_Y) && only_ctrl)
            {
                state.editor.redo();
            }
            else if (keyboard::pressed(ctx->keyboard, SCANCODE_TAB) && without_ctrls)
            {
                state.editor.insert("\t");
            }
            else if (mouse::inside(ctx->mouse, 8, 11, 120, 128) && ctx->mouse.z != 0)
            {
                auto min_y = 0;
                auto max_y = std::max(0, static_cast<int>(state.editor.line_count()) - 14);

                if (ctx->mouse.z < 0)
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

        if (mouse::inside(ctx->mouse, 0, 11, 128, 128) || state.dragging)
        {
            const auto local_x = ctx->mouse.x - 8 + state.scroll_x * 4;
            const auto local_y = ctx->mouse.y - 11 + state.scroll_y * 8;

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
                const auto ch = state.editor.get_char(line_start + col);
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

            if (mouse::pressed(ctx->mouse, 1))
            {
                state.editor.collapse(cursor_index);
                state.dragging = true;
                auto_scroll = true;
            }
            else if (mouse::down(ctx->mouse, 1))
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

    void draw(AppContext *ctx, EditorState &state)
    {
        gfx::draw_rect(ctx->memory, 0, 11, 8, 117, 14, true);
        gfx::draw_rect(ctx->memory, 8, 11, 120, 117, 15, true);

        const auto sel_start = state.editor.sel_start();
        const auto sel_end = state.editor.sel_end();
        const auto cursor = state.editor.cursor();

        gfx::set_offset(ctx->memory, 8, 11);
        gfx::set_clip(ctx->memory, 8, 11, 120, 117);

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
                const auto ch = state.editor.get_char(line_start + col);
                const auto current_index = line_start + col;
                if ((ctx->timer.ticks() >> 5) % 2 && current_index == cursor)
                    gfx::draw_rect(ctx->memory, x, y, 1, 8, 3, true);

                if (current_index >= sel_start && current_index < sel_end && ch != '\n')
                    gfx::draw_rect(ctx->memory, x, y, ch == '\t' ? 8 : 4, 8, 3, true);

                if (ch == '\t')
                {
                    x += 8;
                }
                else if (ch != '\n')
                {
                    gfx::draw_char(ctx->memory, ch, x, y, 1);
                    x += 4;
                }
            }

            if ((ctx->timer.ticks() >> 5) % 2 && line == below - 1 && line_start + line_size == cursor)
                gfx::draw_rect(ctx->memory, x, y, 1, 8, 3, true);

            x = -state.scroll_x * 4;
            y += 8;
        }

        gfx::set_offset(ctx->memory);
        gfx::set_clip(ctx->memory);
    }
}