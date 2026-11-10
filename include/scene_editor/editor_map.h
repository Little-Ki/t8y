#pragma once
#include <cstdint>

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::editor::map
{
    using namespace t8::core;

    struct EditorState
    {
        uint8_t sprite_id = 0;
        uint8_t zoom = 1;
        uint8_t page = 0;
        int x = 0;
        int y = 11;
    };

    void update(AppContext *ctx, EditorState &state);

    void draw(AppContext *ctx, EditorState &state);
}