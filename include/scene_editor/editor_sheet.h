#pragma once

#include <functional>

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::editor::sheet
{
    using namespace t8::core;

    enum class EditorTool
    {
        Pencil,
        Straw,
        Barrel
    };

    struct DrawTarget
    {
        std::function<void(int, int, uint8_t)> set;
        std::function<uint8_t(int, int)> get;
    };

    struct EditorState
    {
        EditorTool tool = EditorTool::Pencil;
        uint8_t sprite_id = 0;
        uint8_t color = 1;
        uint8_t page = 0;
        uint8_t zoom = 1;
        uint8_t pencil_size = 1;
        bool font_mode = false;
    };

    void update(AppContext *ctx, EditorState &state);

    void draw(AppContext *ctx, EditorState &state);
}