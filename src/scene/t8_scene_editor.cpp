#include "t8_scene_editor.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keyboard.h"
#include "t8_input_mouse.h"
#include "t8_utils_timer.h"

#include "t8_scene_editor_map.h"
#include "t8_scene_editor_script.h"
#include "t8_scene_editor_sheet.h"

#include <functional>

using namespace t8::core;
using namespace t8::input;
using namespace t8::utils;

namespace t8::scene {

    enum class EditorPage {
        Script,
        Sprite,
        Map
    };

    EditorPage page = EditorPage::Script;

    void editor_draw_tab() {
        painter_rect(0, 0, 128, 11, 13, true);
        painter_char(0x80, 1, 1, (page == EditorPage::Script || mouse_inside(2, 2, 7, 7)) ? 7 : 1);
        painter_char(0x81, 10, 1, (page == EditorPage::Sprite || mouse_inside(11, 2, 7, 7)) ? 7 : 1);
        painter_char(0x83, 19, 1, (page == EditorPage::Map || mouse_inside(20, 2, 7, 7)) ? 7 : 1);
    }

    void editor_update_tab() {
        if (mouse_clicked(1, 1, 8, 8)) {
            ctx_signals().push({SIGNAL_START_INPUT});
            page = EditorPage::Script;
        }
        if (mouse_clicked(10, 1, 8, 8)) {
            ctx_signals().push({SIGNAL_STOP_INPUT});
            page = EditorPage::Sprite;
        }
        if (mouse_clicked(19, 1, 8, 8)) {
            ctx_signals().push({SIGNAL_STOP_INPUT});
            page = EditorPage::Map;
        }
    }

    void editor_update() {
        if (keyboard_pressed(41)) {
            ctx_signals().push({SIGNAL_SWAP_CONSOLE});
            return;
        }
        editor_update_tab();
        if (page == EditorPage::Script) {
            update_script_editor();
        }
        if (page == EditorPage::Sprite) {
            update_steet_editor();
        }
        if (page == EditorPage::Map) {
            update_map_editor();
        }
    }

    void editor_draw() {
        painter_clear(0);
        editor_draw_tab();

        if (page == EditorPage::Script) {
            draw_script_editor();
        }
        if (page == EditorPage::Sprite) {
            draw_sheet_editor();
        }
        if (page == EditorPage::Map) {
            draw_map_editor();
        }
    }

    void editor_enter() {
        painter_reset();

        script_editor_set_script(ctx_script());

        if (page == EditorPage::Script) {
            ctx_signals().push({SIGNAL_START_INPUT});
        }

        timer_reset();
    }

    void editor_leave() {
        ctx_script() = script_editor_get_script();
        ctx_signals().push({SIGNAL_STOP_INPUT});
    }
}