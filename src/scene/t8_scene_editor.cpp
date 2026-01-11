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

    struct ScriptEditorState {
        int scroll_x;
        int scroll_y;
        int visual_column;
        t8::text_editor::TextEditor editor;
    };

    struct SheetEditorState {
        EditorTool tool = EditorTool::Pencil;
        uint8_t sprite_id = 0;
        uint8_t color = 1;
        uint8_t page = 0;
        uint8_t zoom = 1;
        uint8_t pencil_size = 1;
        bool font_mode = false;
        int scroll_x;
        int scroll_y;
        int visual_column;
    };

    struct MapEditorState {
        uint8_t sprite_id = 0;
        uint8_t zoom = 1;
        int x = 0;
        int y = 11;
    };

    EditorPage page = EditorPage::Sprite;
    ScriptEditorState script_state;
    SheetEditorState sheet_state;
    MapEditorState map_state;

    void editor_draw_tab() {
        painter_rect(0, 0, 128, 11, 13, true);
        painter_char(0x80, 1, 1, (page == EditorPage::Script || mouse_inside(2, 2, 7, 7)) ? 7 : 1);
        painter_char(0x81, 10, 1, (page == EditorPage::Sprite || mouse_inside(11, 2, 7, 7)) ? 7 : 1);
        painter_char(0x83, 19, 1, (page == EditorPage::Map || mouse_inside(20, 2, 7, 7)) ? 7 : 1);
    }

    void editor_update_tab() {
        if (mouse_clicked(1, 1, 8, 8)) {
            signal_push({SIGNAL_START_INPUT});
            page = EditorPage::Script;
        }
        if (mouse_clicked(10, 1, 8, 8)) {
            signal_push({SIGNAL_STOP_INPUT});
            page = EditorPage::Sprite;
        }
        if (mouse_clicked(19, 1, 8, 8)) {
            signal_push({SIGNAL_STOP_INPUT});
            page = EditorPage::Map;
        }
    }

    void editor_draw_sheet_edit_tools() {
        // 画板工具按钮
        painter_char(0x90, 7, 85, (sheet_state.tool == EditorTool::Pencil || mouse_inside(7, 85, 8, 8)) ? 3 : 1);
        painter_char(0x91, 15, 85, (sheet_state.tool == EditorTool::Straw || mouse_inside(15, 85, 8, 8)) ? 3 : 1);
        painter_char(0x92, 23, 85, (sheet_state.tool == EditorTool::Barrel || mouse_inside(23, 85, 8, 8)) ? 3 : 1);
        painter_char(0x93, 31, 85, mouse_inside(31, 85, 8, 8) ? 3 : 1);
        painter_char(0x94, 39, 85, mouse_inside(39, 85, 8, 8) ? 3 : 1);
        painter_char(0x95, 47, 85, mouse_inside(47, 85, 8, 8) ? 3 : 1);
        painter_char(0x96, 55, 85, mouse_inside(55, 85, 8, 8) ? 3 : 1);
    }
    void editor_draw_sheet_edit_id() {
        const auto sprite_id = sheet_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const static char hex[16] = {
            '0', '1', '2', '3',
            '4', '5', '6', '7',
            '8', '9', 'A', 'B',
            'C', 'D', 'E', 'F'};

        painter_char(hex[id_y], 64, 86, 3);
        painter_char(hex[id_x], 68, 86, 3);
    }

    void editor_draw_sheet_edit_board() {
        const auto sprite_id = sheet_state.sprite_id;
        const auto sprite_x = (sprite_id & 0xF) * 8;
        const auto sprite_y = ((sprite_id >> 4) & 0xF) * 8;
        auto pixel_size = 8 / sheet_state.zoom;
        auto range = sheet_state.zoom * 8;

        painter_rect(7, 18, 66, 66, 0);
        for (auto dy = 0; dy < range; dy++) {
            for (auto dx = 0; dx < range; dx++) {
                auto color = sheet_state.font_mode ? painter_font(sprite_x + dx, sprite_y + dy, true) : painter_sprite(sprite_x + dx, sprite_y + dy);
                painter_rect(8 + dx * pixel_size, 19 + dy * pixel_size, pixel_size, pixel_size, color, true);
            }
        }
    }

    void editor_draw_sheet_edit_flags() {
        const auto sprite_id = sheet_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        if (!sheet_state.font_mode) {
            auto flag = 0;
            for (auto y = id_y; y < std::min(16, id_y + sheet_state.zoom); y++) {
                for (auto x = id_x; x < std::min(16, id_x + sheet_state.zoom); x++) {
                    flag |= painter_flags(((y << 4) | x) & 0xFF);
                }
            }
            for (auto i = 0; i < 8; i++) {
                painter_rect(85 + ((i << 2) + i), 80, 4, 4, 0, true);
                if (flag & (1 << i)) {
                    painter_rect(86 + ((i << 2) + i), 81, 2, 2, 3, true);
                }
            }
        }
    }

    void editor_draw_sheet_edit_palette() {
        painter_rect(87, 18, 34, 34, 0);
        for (auto i = 0; i < 16; i++) {
            int x = i & 0b11;
            int y = i >> 2;
            painter_rect(88 + (x << 3), 19 + (y << 3), 8, 8, i, true);
        }

        auto select = sheet_state.color;
        int x = select & 0b11;
        int y = select >> 2;
        painter_rect(87 + (x << 3), 18 + (y << 3), 10, 10, 1);
    }

    void editor_draw_sheet_edit_scroller() {
        painter_rect(100, 61, 17, 1, 13);
        painter_rect(100, 71, 17, 1, 13);

        for (auto i = 0; i < 3; i++) {
            painter_rect(99 + (i << 3), 60, 3, 3, 0, true);
            if (sheet_state.zoom == (1 << i)) {
                painter_pixel(100 + (i << 3), 61, 3);
            }

            painter_rect(99 + (i << 3), 70, 3, 3, 0, true);
            if (sheet_state.pencil_size == (((1 << i) >> 1) << 1) + 1) {
                painter_pixel(100 + (i << 3), 71, 3);
            }
        }

        painter_rect(88, 68, 7, 7, 0, true);

        painter_rect(
            91 - (sheet_state.pencil_size >> 1),
            71 - (sheet_state.pencil_size >> 1),
            sheet_state.pencil_size,
            sheet_state.pencil_size,
            1, true);

        painter_char(0x98, 87, 57, 1);
    }

    void editor_draw_sheet_edit_pages() {
        for (auto i = 0; i < 4; i++) {
            painter_rect(99 + i * 6, 87, 7, 6, 13);
            painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
        }

        painter_rect(98 + sheet_state.page * 6, 86, 9, 8, 13);
        painter_rect(99 + sheet_state.page * 6, 87, 7, 6, 3, true);
    }

    void editor_draw_sheet_edit_sprites() {
        const auto sprite_id = sheet_state.sprite_id;
        const auto sprite_x = (sprite_id & 0xF) * 8;
        const auto sprite_y = ((sprite_id >> 4) & 0xF) * 8;
        for (auto dy = 0; dy < 32; dy++) {
            for (auto dx = 0; dx < 128; dx++) {
                auto color = sheet_state.font_mode ? painter_font(dx, (sheet_state.page << 5) + dy, true) : painter_sprite(dx, (sheet_state.page << 5) + dy);
                painter_pixel(dx, 96 + dy, color);
            }
        }

        painter_clip(0, 96, 128, 32);
        // 选择的区域
        painter_rect(
            sprite_x,
            96 + sprite_y - (sheet_state.page << 5),
            sheet_state.zoom << 3,
            sheet_state.zoom << 3,
            1);
        painter_clip();
    }

    void editor_draw_sheet_edit() {
        painter_rect(0, 11, 128, 84, 14, true);
        editor_draw_sheet_edit_board();
        editor_draw_sheet_edit_palette();
        editor_draw_sheet_edit_tools();
        editor_draw_sheet_edit_id();
        editor_draw_sheet_edit_flags();
        editor_draw_sheet_edit_scroller();
        editor_draw_sheet_edit_pages();
        editor_draw_sheet_edit_sprites();
        // 启用字体编辑
        painter_char(0x82, 84, 85, (sheet_state.font_mode || mouse_inside(85, 86, 7, 7)) ? 6 : 1);
    }

    void editor_update() {
        if (keyboard_pressed(41)) {
            signal_push(SIGNAL_SWAP_CONSOLE);
            return;
        }
        editor_update_tab();
    }

    void editor_draw() {
        painter_clear(0);
        editor_draw_tab();

        if (page == EditorPage::Sprite) {
            editor_draw_sheet_edit();
        }
        if (page == EditorPage::Script) {
        }
        if (page == EditorPage::Map) {
        }
    }

    void editor_enter() {
        painter_reset();

        script_state.editor.reset(get_script());

        if (page == EditorPage::Script) {
            signal_push(SIGNAL_START_INPUT);
        }

        timer_reset();
    }

    void editor_leave() {
        set_script(script_state.editor.to_string());
        signal_push(SIGNAL_STOP_INPUT);
    }

}