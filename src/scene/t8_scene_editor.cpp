#include "t8_scene_editor.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keyboard.h"
#include "t8_input_mouse.h"

#include <functional>

using namespace t8::core;
using namespace t8::utils;
using namespace t8::input;

namespace t8::scene {

    enum class EditorPage {
        Script,
        Sprite,
        Map
    };

    enum class EditorTool {
        Pencil,
        Straw,
        Barrel
    };

    struct EditorState {
        EditorPage page = EditorPage::Sprite;

        struct {
            int scroll_x;
            int scroll_y;
            int visual_column;
            t8::text_editor::TextEditor editor;
        } script;

        struct {
            EditorTool tool = EditorTool::Pencil;
            uint8_t sprite_id = 0;
            uint8_t color = 1;
            uint8_t page = 0;
            bool font_mode = false;
            int scroll_x;
            int scroll_y;
            int visual_column;
        } sprite;

        struct {
            uint8_t sprite_id = 0;
            uint8_t zoom = 1;
            int x = 0;
            int y = 11;
        } map;
    };

    EditorState state;

    void editor_update() {
        if (keyboard_pressed(41)) {
            signal_push(SIGNAL_SWAP_CONSOLE);
            return;
        }
    }

    void editor_draw() {
        painter_clear(0);
    }

    void editor_enter() {
        painter_reset();

        state.script.editor.reset(get_script());

        if (state.page == EditorPage::Script) {
            signal_push(SIGNAL_START_INPUT);
        }

        timer_reset();
    }

    void editor_leave() {
        set_script(state.script.editor.to_string());
        signal_push(SIGNAL_STOP_INPUT);
    }

}