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
    struct EditorState {
        uint8_t sprite_id = 0;
        uint8_t zoom = 1;
        uint8_t page = 0;
        int x = 0;
        int y = 11;
    };

    static EditorState state;

    void update_map_editor() {
        // 选择分组
        for (auto i = 0; i < 4; i++) {
            if (mouse_clicked(100 + i * 6, 88, 5, 4)) {
                state.page = i;
            }
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32)) {
            auto local_x = mouse_x();
            auto local_y = mouse_y() - 96;
            local_x >>= 3;
            local_y >>= 3;
            auto next_id = ((local_y << 4) | (local_x & 0xF)) & 0xFF;
            next_id += (state.page << 6);
            state.sprite_id = next_id;
        } else if (mouse_inside(0, 96, 128, 32)) {
            if (mouse_z() < 0) {
                state.page = std::clamp(state.page + 1, 0, 3);
            }
            if (mouse_z() > 0) {
                state.page = std::clamp(state.page - 1, 0, 3);
            }
        }

        // 绘制地图
        if (mouse_dragging(0, 11, 128, 74)) {
            auto local_x = mouse_x() - state.x;
            auto local_y = mouse_y() - state.y;
            local_x /= 8 >> (state.zoom >> 1);
            local_y /= 8 >> (state.zoom >> 1);
            painter_map(local_x, local_y, state.sprite_id);
        } else if (mouse_dragging(0, 11, 128, 74, 2)) {
            state.x += mouse_dx();
            state.y += mouse_dy();
        } else if (mouse_inside(0, 11, 128, 74)) {
            const auto z = mouse_z();
            if (z != 0) {
                const auto next = std::clamp(z < 0 ? state.zoom << 1 : state.zoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != state.zoom) {
                    state.x = 64 + static_cast<int>((state.x - 64) * scale);
                    state.y = 48 + static_cast<int>((state.y - 48) * scale);
                    state.zoom = next;
                }
            }
        }
    }

    void draw_map_editor() {
        const auto sprite_id = state.sprite_id;
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

            painter_rect(98 + state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + state.page * 6, 87, 7, 6, 3, true);
        }

        // 下方精灵图预览及选择
        {

            for (auto dy = 0; dy < 32; dy++) {
                for (auto dx = 0; dx < 128; dx++) {
                    auto color = painter_sprite(dx, (state.page << 5) + dy);
                    painter_pixel(dx, 96 + dy, color);
                }
            }

            painter_clip(0, 96, 128, 32);
            // 选择的区域
            painter_rect(
                sprite_x, 96 + sprite_y - (state.page << 5),
                8, 8, 1);
            painter_clip();
        }

        {
            painter_clip(0, 11, 128, 74);

            auto chunk_size = 8 >> (state.zoom >> 1);
            auto step = state.zoom;

            painter_rect(state.x - 1, state.y - 1, 128 * chunk_size + 2, 128 * chunk_size + 2, 2);

            auto l = std::clamp(-state.x / chunk_size, 0, 128);
            auto t = std::clamp(-state.y / chunk_size, 0, 128);
            auto r = std::clamp((128 - state.x) / chunk_size + 1, 0, 128);
            auto b = std::clamp((128 - state.y) / chunk_size + 1, 0, 128);

            for (auto y = t; y < b; y++) {
                for (auto x = l; x < r; x++) {
                    auto id = painter_map(x, y);
                    auto spX = (id & 0xF) << 3;
                    auto spY = ((id >> 4) & 0xF) << 3;

                    for (auto inY = 0; inY < chunk_size; inY++) {
                        for (auto inX = 0; inX < chunk_size; inX++) {
                            auto color = id == 0 ? 0 : painter_sprite(spX + inX * step, spY + inY * step);
                            painter_pixel(
                                state.x + x * chunk_size + inX,
                                state.y + y * chunk_size + inY,
                                color);
                        }
                    }
                }
            }

            if (mouse_inside(0, 11, 128, 74)) {
                auto innerX = mouse_x() - state.x;
                auto innerY = mouse_y() - state.y;
                innerX /= chunk_size;
                innerY /= chunk_size;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128) {
                    painter_rect(state.x + innerX * chunk_size - 1, state.y + innerY * chunk_size - 1, chunk_size + 2, chunk_size + 2, 1);
                }
            }

            painter_clip();
        }
    }
}