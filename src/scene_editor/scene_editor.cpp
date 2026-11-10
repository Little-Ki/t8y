#include "scene_editor.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_mouse.h"
#include "input_keyboard.h"

#include "constants.h"

using namespace t8::input;

namespace t8::scene::editor
{

    static void update_tab(AppContext *ctx, EditorState *state)
    {
        if (mouse::clicked(ctx->mouse, 1, 1, 8, 8))
        {
            ctx->signals.push({SIGNAL_START_INPUT});
            state->tab = EditorTab::Script;
        }
        if (mouse::clicked(ctx->mouse, 10, 1, 8, 8))
        {
            ctx->signals.push({SIGNAL_STOP_INPUT});
            state->tab = EditorTab::Sheet;
        }
        if (mouse::clicked(ctx->mouse, 19, 1, 8, 8))
        {
            ctx->signals.push({SIGNAL_STOP_INPUT});
            state->tab = EditorTab::Map;
        }
    }

    static void draw_tab(AppContext *ctx, EditorState *state)
    {
        gfx::draw_rect(ctx->memory, 0, 0, 128, 11, 13, true);
        gfx::draw_char(ctx->memory, 0x80, 1, 1, (state->tab == EditorTab::Script || mouse::inside(ctx->mouse, 2, 2, 7, 7)) ? 7 : 1);
        gfx::draw_char(ctx->memory, 0x81, 10, 1, (state->tab == EditorTab::Sheet || mouse::inside(ctx->mouse, 11, 2, 7, 7)) ? 7 : 1);
        gfx::draw_char(ctx->memory, 0x83, 19, 1, (state->tab == EditorTab::Map || mouse::inside(ctx->mouse, 20, 2, 7, 7)) ? 7 : 1);
    }

    void update(AppContext *ctx, EditorState *state)
    {
        if (keyboard::pressed(ctx->keyboard, 41))
        {
            ctx->signals.push({SIGNAL_SWAP_CONSOLE});
            return;
        }
        update_tab(ctx, state);

        if (state->tab == EditorTab::Script)
        {
            script::update(ctx, state->script);
        }
        if (state->tab == EditorTab::Sheet)
        {
            sheet::update(ctx, state->sheet);
        }
        if (state->tab == EditorTab::Map)
        {
            map::update(ctx, state->map);
        }
    }

    void draw(AppContext *ctx, EditorState *state)
    {
        gfx::clear(ctx->memory, 0);
        draw_tab(ctx, state);

        if (state->tab == EditorTab::Script)
        {
            script::draw(ctx, state->script);
        }
        if (state->tab == EditorTab::Sheet)
        {
            sheet::draw(ctx, state->sheet);
        }
        if (state->tab == EditorTab::Map)
        {
            map::draw(ctx, state->map);
        }
    }

    void enter(AppContext *ctx, EditorState *state)
    {
        gfx::reset_all(ctx->memory);
        state->script.editor.reset(ctx->script);

        if (state->tab == EditorTab::Script)
        {
            ctx->signals.push({SIGNAL_START_INPUT});
        }

        ctx->timer.reset();
    }

    void leave(AppContext *ctx, EditorState *state)
    {
        ctx->script = state->script.editor.to_string();
        ctx->signals.push({SIGNAL_STOP_INPUT});
    }
}
