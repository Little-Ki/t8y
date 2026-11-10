#pragma once

#include "ted/editor.hpp"

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::editor::script
{
    using namespace t8::core;

    struct EditorState
    {
        int scroll_x = 0;
        int scroll_y = 0;
        bool dragging = false;
        t8::ted::TextEditor editor;
    };

    void update(AppContext *ctx, EditorState &state);

    void draw(AppContext *ctx, EditorState &state);
}