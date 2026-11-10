#pragma once

#include "editor_map.h"
#include "editor_script.h"
#include "editor_sheet.h"

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::editor
{
    using namespace t8::core;

    enum class EditorTab
    {
        Script,
        Sheet,
        Map
    };

    struct EditorState
    {
        EditorTab tab = EditorTab::Script;
        sheet::EditorState sheet;
        script::EditorState script;
        map::EditorState map;
    };

    void update(AppContext *ctx, EditorState *state);

    void draw(AppContext *ctx, EditorState *state);

    void enter(AppContext *ctx, EditorState *state);

    void leave(AppContext *ctx, EditorState *state);
}