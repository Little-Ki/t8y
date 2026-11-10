#include "editor_map.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_mouse.h"

#include <algorithm>

using namespace t8::input;

namespace t8::scene::editor::map
{
    void update(AppContext *ctx, EditorState &state)
    {
        // 选择分组
        for (auto i = 0; i < 4; i++)
        {
            if (mouse::clicked(ctx->mouse, 100 + i * 6, 88, 5, 4))
            {
                state.page = i;
            }
        }

        // 选择精灵
        if (mouse::dragging(ctx->mouse, 0, 96, 128, 32))
        {
            auto local_x = ctx->mouse.x;
            auto local_y = ctx->mouse.y - 96;
            local_x >>= 3;
            local_y >>= 3;
            auto next_id = ((local_y << 4) | (local_x & 0xF)) & 0xFF;
            next_id += (state.page << 6);
            state.sprite_id = next_id;
        }
        else if (mouse::inside(ctx->mouse, 0, 96, 128, 32) && ctx->mouse.z != 0)
        {
            if (ctx->mouse.z < 0)
            {
                state.page = std::clamp(state.page + 1, 0, 3);
            }
            else
            {
                state.page = std::clamp(state.page - 1, 0, 3);
            }
        }

        // 绘制地图
        if (mouse::dragging(ctx->mouse, 0, 11, 128, 74))
        {
            auto local_x = ctx->mouse.x - state.x;
            auto local_y = ctx->mouse.y - state.y;
            local_x /= 8 >> (state.zoom >> 1);
            local_y /= 8 >> (state.zoom >> 1);
            gfx::set_tilemap(ctx->memory, local_x, local_y, state.sprite_id);
        }
        else if (mouse::dragging(ctx->mouse, 0, 11, 128, 74, 2))
        {
            state.x += ctx->mouse.dx;
            state.y += ctx->mouse.dy;
        }
        else if (mouse::inside(ctx->mouse, 0, 11, 128, 74))
        {
            const auto z = ctx->mouse.z;
            if (z != 0)
            {
                const auto next = std::clamp(z < 0 ? state.zoom << 1 : state.zoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != state.zoom)
                {
                    state.x = 64 + static_cast<int>((state.x - 64) * scale);
                    state.y = 48 + static_cast<int>((state.y - 48) * scale);
                    state.zoom = next;
                }
            }
        }
    }

    void draw(AppContext *ctx, EditorState &state)
    {
        const auto sprite_id = state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x << 3;
        const auto sprite_y = id_y << 3;

        gfx::draw_rect(ctx->memory, 0, 11, 128, 84, 10, true);

        // 网格背景
        for (auto y = 11; y < 85; y++)
        {
            for (auto x = 0; x < 128; x++)
            {
                if ((x + y) & 1)
                {
                    gfx::set_pixel(ctx->memory, x, y, 8);
                }
                else
                {
                    gfx::set_pixel(ctx->memory, x, y, 0);
                }
            }
        }

        gfx::draw_rect(ctx->memory, 0, 85, 128, 10, 14, true);

        {
            for (auto i = 0; i < 4; i++)
            {
                gfx::draw_rect(ctx->memory, 99 + i * 6, 87, 7, 6, 13);
                gfx::draw_rect(ctx->memory, 100 + i * 6, 88, 5, 4, mouse::inside(ctx->mouse, 100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            gfx::draw_rect(ctx->memory, 98 + state.page * 6, 86, 9, 8, 13);
            gfx::draw_rect(ctx->memory, 99 + state.page * 6, 87, 7, 6, 3, true);
        }

        // 下方精灵图预览及选择
        {

            for (auto dy = 0; dy < 32; dy++)
            {
                for (auto dx = 0; dx < 128; dx++)
                {
                    auto color = gfx::get_sprite(ctx->memory, dx, (state.page << 5) + dy);
                    gfx::set_pixel(ctx->memory, dx, 96 + dy, color);
                }
            }

            gfx::set_clip(ctx->memory, 0, 96, 128, 32);
            // 选择的区域
            gfx::draw_rect(
                ctx->memory,
                sprite_x, 96 + sprite_y - (state.page << 5),
                8, 8, 1);
            gfx::set_clip(ctx->memory);
        }

        {
            gfx::set_clip(ctx->memory, 0, 11, 128, 74);

            auto chunk_size = 8 >> (state.zoom >> 1);
            auto step = state.zoom;

            gfx::draw_rect(ctx->memory, state.x - 1, state.y - 1, 128 * chunk_size + 2, 128 * chunk_size + 2, 2);

            auto l = std::clamp(-state.x / chunk_size, 0, 128);
            auto t = std::clamp(-state.y / chunk_size, 0, 128);
            auto r = std::clamp((128 - state.x) / chunk_size + 1, 0, 128);
            auto b = std::clamp((128 - state.y) / chunk_size + 1, 0, 128);

            for (auto y = t; y < b; y++)
            {
                for (auto x = l; x < r; x++)
                {
                    auto id = gfx::get_tilemap(ctx->memory, x, y);
                    auto sp_x = (id & 0xF) << 3;
                    auto sp_y = ((id >> 4) & 0xF) << 3;

                    for (auto dy = 0; dy < chunk_size; dy++)
                    {
                        for (auto dx = 0; dx < chunk_size; dx++)
                        {
                            auto color = id == 0 ? 0 : gfx::get_sprite(ctx->memory, sp_x + dx * step, sp_y + dy * step);
                            gfx::set_pixel(
                                ctx->memory,
                                state.x + x * chunk_size + dx,
                                state.y + y * chunk_size + dy,
                                color);
                        }
                    }
                }
            }

            if (mouse::inside(ctx->mouse, 0, 11, 128, 74))
            {
                auto innerX = ctx->mouse.x - state.x;
                auto innerY = ctx->mouse.y - state.y;
                innerX /= chunk_size;
                innerY /= chunk_size;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128)
                {
                    gfx::draw_rect(ctx->memory, state.x + innerX * chunk_size - 1, state.y + innerY * chunk_size - 1, chunk_size + 2, chunk_size + 2, 1);
                }
            }

            gfx::set_clip(ctx->memory);
        }
    }
}