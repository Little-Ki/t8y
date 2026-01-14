#include "t8_scene_editor_map.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keyboard.h"
#include "t8_input_mouse.h"
#include "t8_utils_timer.h"

#include "t8_text_editor.hpp"

using namespace t8::core;
using namespace t8::utils;
using namespace t8::input;

namespace t8::scene {
    struct MapEditorState {
        uint8_t sprite_id = 0;
        uint8_t zoom = 1;
        uint8_t page = 0;
        int x = 0;
        int y = 11;
    };

    MapEditorState map_state;

    void update_map_editor() {
        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (mouse_clicked(100 + i * 6, 88, 5, 4)) {
                map_state.page = i;
            }
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32)) {
            auto local_x = mouse_x();
            auto local_y = mouse_y() - 96;
            local_x >>= 3;
            local_y >>= 3;
            auto next_id = ((local_y << 4) | (local_x & 0xF)) & 0xFF;
            next_id += (map_state.page << 6);
            map_state.sprite_id = next_id;
        } else if (mouse_inside(0, 96, 128, 32)) {
            if (mouse_z() < 0) {
                map_state.page = std::clamp(map_state.page + 1, 0, 3);
            }
            if (mouse_z() > 0) {
                map_state.page = std::clamp(map_state.page - 1, 0, 3);
            }
        }

        // 绘制地图
        if (mouse_dragging(0, 11, 128, 74)) {
            auto local_x = mouse_x() - map_state.x;
            auto local_y = mouse_y() - map_state.y;
            local_x /= 8 >> (map_state.zoom >> 1);
            local_y /= 8 >> (map_state.zoom >> 1);
            painter_map(local_x, local_y, map_state.sprite_id);
        } else if (mouse_dragging(0, 11, 128, 74, 2)) {
            map_state.x += mouse_dx();
            map_state.y += mouse_dy();
        } else if (mouse_inside(0, 11, 128, 74)) {
            const auto z = mouse_z();
            if (z != 0) {
                const auto next = std::clamp(z < 0 ? map_state.zoom << 1 : map_state.zoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != map_state.zoom) {
                    map_state.x = 64 + static_cast<int>((map_state.x - 64) * scale);
                    map_state.y = 48 + static_cast<int>((map_state.y - 48) * scale);
                    map_state.zoom = next;
                }
            }
        }
    }

    void draw_map_editor() {
        const auto sprite_id = map_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x << 3;
        const auto sprite_y = id_y << 3;

        painter_rect(0, 11, 128, 84, 10, true);

        // 网格背景
        for (auto y = 11; y < 85; y++) {
            for (auto x = 0; x < 128; x++) {
                if ((x + y) & 1) {
                    painter_pixel(x, y, 8);
                } else {
                    painter_pixel(x, y, 0);
                }
            }
        }

        painter_rect(0, 85, 128, 10, 14, true);

        {
            for (auto i = 0; i < 4; i++) {
                painter_rect(99 + i * 6, 87, 7, 6, 13);
                painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            painter_rect(98 + map_state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + map_state.page * 6, 87, 7, 6, 3, true);
        }

        // 下方精灵图预览及选择
        {

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = painter_sprite(dx, (map_state.page << 5) + dy);
                    painter_pixel(dx, 96 + dy, color);
                }
            }

            painter_clip(0, 96, 128, 32);
            // 选择的区域
            painter_rect(
                sprite_x, 96 + sprite_y - (map_state.page << 5),
                8, 8, 1);
            painter_clip();
        }

        {
            painter_clip(0, 11, 128, 74);

            auto chunk_size = 8 >> (map_state.zoom >> 1);
            auto step = map_state.zoom;

            painter_rect(map_state.x - 1, map_state.y - 1, 128 * chunk_size + 2, 128 * chunk_size + 2, 2);

            auto l = std::clamp(-map_state.x / chunk_size, 0, 128);
            auto t = std::clamp(-map_state.y / chunk_size, 0, 128);
            auto r = std::clamp((128 - map_state.x) / chunk_size + 1, 0, 128);
            auto b = std::clamp((128 - map_state.y) / chunk_size + 1, 0, 128);

            for (auto y = t; y < b; y++) {
                for (auto x = l; x < r; x++) {
                    auto id = painter_map(x, y);
                    auto spX = (id & 0xF) << 3;
                    auto spY = ((id >> 4) & 0xF) << 3;

                    for (auto inY = 0; inY < chunk_size; inY++) {
                        for (auto inX = 0; inX < chunk_size; inX++) {
                            auto color = id == 0 ? 0 : painter_sprite(spX + inX * step, spY + inY * step);
                            painter_pixel(
                                map_state.x + x * chunk_size + inX,
                                map_state.y + y * chunk_size + inY,
                                color);
                        }
                    }
                }
            }

            if (mouse_inside(0, 11, 128, 74)) {
                auto innerX = mouse_x() - map_state.x;
                auto innerY = mouse_y() - map_state.y;
                innerX /= chunk_size;
                innerY /= chunk_size;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128) {
                    painter_rect(map_state.x + innerX * chunk_size - 1, map_state.y + innerY * chunk_size - 1, chunk_size + 2, chunk_size + 2, 1);
                }
            }

            painter_clip();
        }
    }
}