#pragma once
#include "t8_textedit.h";
#include "t8_timer.h"

namespace t8 {

    enum class EditorTab {
        Script,
        Sheet,
        Map
    };

    enum class EditorTool {
        Pencil,
        Straw,
        Barrel
    };

    struct EditorState {
        int view_size = 1;
        int pencil_size = 1;
        int page = 0;
        uint8_t id = 0;
        uint8_t color = 0;
        EditorTool tool = EditorTool::Pencil;
        bool edit_font = false;
        
        int map_x = 0;
        int map_y = 11;
        int map_zoom = 1;

        int edit_x = 0;
        int edit_y = 0;

        EditorTab tab = EditorTab::Sheet;

        TextEdit editor;

        Timer timer;
    };

    void editor_update();

    void editor_draw();

    void editor_enter();

    void editor_leave();

}