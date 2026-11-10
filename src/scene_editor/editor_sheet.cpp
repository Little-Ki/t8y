#include "editor_sheet.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_mouse.h"
#include "input_keyboard.h"

#include "constants.h"

#include <stack>
#include <algorithm>

using namespace t8::input;

namespace t8::scene::editor::sheet
{

    void spray_sheet(
        int area_x, int area_y, int area_size,
        int start_x, int start_y,
        uint8_t replace, uint8_t color,
        const DrawTarget &target)
    {
        int l = std::max(area_x, 0);
        int t = std::max(area_y, 0);
        int r = std::min(area_x + area_size, 128);
        int b = std::min(area_y + area_size, 128);

        std::stack<std::tuple<int, int>> points;

        points.push({start_x, start_y});

        while (!points.empty())
        {
            const auto p = points.top();
            const auto x = std::get<0>(p);
            const auto y = std::get<1>(p);

            points.pop();

            if (x < l || x >= r || y < t || y >= b)
                continue;

            const auto it = target.get(x, y);

            if (it == color)
                continue;

            if (it == replace)
            {
                target.set(x, y, color);
                points.push({x - 1, y});
                points.push({x + 1, y});
                points.push({x, y - 1});
                points.push({x, y + 1});
            }
        }
    }

    void editor_draw_sheet_pixel(
        int area_x, int area_y, int area_size,
        int x, int y, int pixel_size,
        uint8_t color,
        const DrawTarget &target)
    {
        auto l = std::max(x - (pixel_size >> 1), area_x);
        auto t = std::max(y - (pixel_size >> 1), area_y);
        auto r = std::min(x + (pixel_size >> 1) + 1, area_x + area_size);
        auto b = std::min(y + (pixel_size >> 1) + 1, area_y + area_size);

        for (auto y = t; y < b; y++)
        {
            for (auto x = l; x < r; x++)
            {
                target.set(x, y, color);
            }
        }
    }

    void editor_erase_sheet(
        int area_x, int area_y, int area_size,
        const DrawTarget &target)
    {
        for (auto y = area_y; y < area_y + area_size; y++)
        {
            for (auto x = area_x; x < area_x + area_size; x++)
            {
                target.set(x, y, 0);
            }
        }
    }

    void editor_rotate_sheet(
        int x, int y, int size,
        const DrawTarget &target)
    {
        const auto half = size >> 1;
        const auto next = [&](int &x, int &y)
        {
            auto t = y;
            y = x;
            x = size - t - 1;
        };

        for (auto dy = 0; dy < half; dy++)
        {
            for (auto dx = 0; dx < half; dx++)
            {
                auto tx = dx, ty = dy;
                auto prev = target.get(x + dx, y + dy);
                for (auto i = 0; i < 4; i++)
                {
                    next(tx, ty);
                    auto it = target.get(x + tx, y + ty);
                    target.set(x + tx, y + ty, prev);
                    prev = it;
                }
            }
        }
    }

    void editor_flip_sheet(
        int area_x, int area_y, int size,
        bool vertical,
        const DrawTarget &target)
    {
        if (vertical)
        {
            for (auto dy = 0; dy < (size >> 1); dy++)
            {
                for (auto dx = 0; dx < size; dx++)
                {
                    auto that = target.get(area_x + dx, area_y + size - dy - 1);
                    auto it = target.get(area_x + dx, area_y + dy);
                    target.set(area_x + dx, area_y + size - dy - 1, it);
                    target.set(area_x + dx, area_y + dy, that);
                }
            }
        }
        else
        {
            for (auto dy = 0; dy < size; dy++)
            {
                for (auto dx = 0; dx < (size >> 1); dx++)
                {
                    auto that = target.get(area_x + size - dx - 1, area_y + dy);
                    auto it = target.get(area_x + dx, area_y + dy);
                    target.set(area_x + size - dx - 1, area_y + dy, it);
                    target.set(area_x + dx, area_y + dy, that);
                }
            }
        }
    }

    void update(AppContext *ctx, EditorState &state)
    {
        const auto sprite_id = state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x << 3;
        const auto sprite_y = id_y << 3;
        const auto area_size = state.zoom << 3;

        const DrawTarget draw_font = {
            [&](int x, int y, uint8_t c)
            { gfx::set_font(ctx->memory, x, y, c, true); },
            [&](int x, int y)
            { return gfx::get_font(ctx->memory, x, y, true); }};

        const DrawTarget draw_sprite = {
            [&](int x, int y, uint8_t c)
            { gfx::set_sprite(ctx->memory, x, y, c); },
            [&](int x, int y)
            { return gfx::get_sprite(ctx->memory, x, y); }};

        const DrawTarget draw_target = state.font_mode ? draw_font : draw_sprite;

        // 工具操作
        {
            if (mouse::clicked(ctx->mouse, 8, 86, 7, 7))
            {
                state.tool = EditorTool::Pencil;
            }
            if (mouse::clicked(ctx->mouse, 16, 86, 7, 7))
            {
                state.tool = EditorTool::Straw;
            }
            if (mouse::clicked(ctx->mouse, 24, 86, 7, 7))
            {
                state.tool = EditorTool::Barrel;
            }
            if (mouse::clicked(ctx->mouse, 32, 86, 7, 7))
            {
                editor_flip_sheet(sprite_x, sprite_y, area_size, false, draw_target);
            }
            if (mouse::clicked(ctx->mouse, 40, 86, 7, 7))
            {
                editor_flip_sheet(sprite_x, sprite_y, area_size, true, draw_target);
            }
            if (mouse::clicked(ctx->mouse, 48, 86, 7, 7))
            {
                editor_rotate_sheet(sprite_x, sprite_y, area_size, draw_target);
            }
            if (mouse::clicked(ctx->mouse, 56, 86, 7, 7))
            {
                editor_erase_sheet(sprite_x, sprite_y, area_size, draw_target);
            }
        }

        // 字体模式
        if (mouse::clicked(ctx->mouse, 85, 86, 7, 7))
        {
            state.font_mode = !state.font_mode;
        }

        // 精灵页面
        for (auto i = 0; i < 4; i++)
        {
            if (mouse::clicked(ctx->mouse, 100 + i * 6, 88, 5, 4))
            {
                state.page = i;
            }
        }

        // 调色板
        if (mouse::dragging(ctx->mouse, 88, 19, 32, 32))
        {
            int x = (ctx->mouse.x - 88) >> 3;
            int y = (ctx->mouse.y - 19) >> 3;
            state.color = (y << 2) | x;
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

        // 右侧滑块
        for (auto i = 0; i < 3; i++)
        {
            if (mouse::clicked(ctx->mouse, 98 + (i << 3), 59, 5, 5))
            {
                state.zoom = (1 << i);
            }
            if (mouse::clicked(ctx->mouse, 98 + (i << 3), 69, 5, 5))
            {
                state.pencil_size = 1 + i * 2;
            }
        }

        // 设置精灵Flag
        if (!state.font_mode)
        {
            auto old = 0;
            for (auto y = id_y; y < ((id_y + state.zoom) & 0xF); y++)
            {
                for (auto x = id_x; x < ((id_x + state.zoom) & 0xF); x++)
                {
                    old |= gfx::get_flags(ctx->memory, ((y << 4) | x) & 0xFF);
                }
            }

            for (auto i = 0; i < 8; i++)
            {
                if (mouse::clicked(ctx->mouse, 85 + ((i << 2) + i), 81, 4, 4))
                {
                    for (auto y = id_y; y < ((id_y + state.zoom) & 0xF); y++)
                    {
                        for (auto x = id_x; x < ((id_x + state.zoom) & 0xF); x++)
                        {
                            auto flag = gfx::get_flags(ctx->memory, ((y << 4) | x) & 0xFF);
                            if (old & (1 << i))
                            {
                                flag &= ~static_cast<uint8_t>(1 << i);
                            }
                            else
                            {
                                flag |= static_cast<uint8_t>(1 << i);
                            }
                            gfx::set_flags(ctx->memory, ((y << 4) | x) & 0xFF, flag);
                        }
                    }
                }
            }
        }

        // 绘制操作
        {

            auto local_x = ctx->mouse.x - 8;
            auto local_y = ctx->mouse.y - 19;

            local_x /= 8 >> (state.zoom >> 1);
            local_y /= 8 >> (state.zoom >> 1);

            if (state.tool == EditorTool::Straw)
            {
                if (mouse::clicked(ctx->mouse, 8, 19, 64, 64))
                {
                    state.color = draw_target.get(sprite_x + local_x, sprite_y + local_y);
                    state.tool = EditorTool::Pencil;
                }
            }

            if (state.tool == EditorTool::Barrel)
            {
                if (mouse::clicked(ctx->mouse, 8, 19, 64, 64))
                {
                    auto replace = draw_target.get(sprite_x + local_x, sprite_y + local_y);

                    spray_sheet(
                        sprite_x, sprite_y,
                        area_size,
                        sprite_x + local_x, sprite_y + local_y,
                        replace,
                        state.color,
                        draw_target);

                    state.tool = EditorTool::Pencil;
                }
            }
            if (state.tool == EditorTool::Pencil)
            {
                if (mouse::dragging(ctx->mouse, 8, 19, 64, 64))
                {
                    editor_draw_sheet_pixel(
                        sprite_x, sprite_y, area_size,
                        sprite_x + local_x, sprite_y + local_y,
                        state.pencil_size, state.color,
                        draw_target);
                }
            }
            if (mouse::inside(ctx->mouse, 8, 19, 64, 64) && ctx->mouse.z != 0)
            {
                if (ctx->mouse.z < 0)
                {
                    state.zoom = std::clamp(state.zoom << 1, 1, 4);
                }
                else
                {
                    state.zoom = std::clamp(state.zoom >> 1, 1, 4);
                }
            }
        }
    }

    void draw(AppContext *ctx, EditorState &state)
    {
        const auto sprite_id = state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x * 8;
        const auto sprite_y = id_y * 8;
        auto pixel_size = 8 / state.zoom;
        auto range = state.zoom * 8;

        gfx::draw_rect(ctx->memory, 0, 11, 128, 84, 14, true);

        // 画板
        gfx::draw_rect(ctx->memory, 7, 18, 66, 66, 0);
        for (auto dy = 0; dy < range; dy++)
        {
            for (auto dx = 0; dx < range; dx++)
            {
                auto color = state.font_mode ? gfx::get_font(ctx->memory, sprite_x + dx, sprite_y + dy, true) : gfx::get_sprite(ctx->memory, sprite_x + dx, sprite_y + dy);
                gfx::draw_rect(ctx->memory, 8 + dx * pixel_size, 19 + dy * pixel_size, pixel_size, pixel_size, color, true);
            }
        }

        //  调色板
        {
            gfx::draw_rect(ctx->memory, 87, 18, 34, 34, 0);
            for (auto i = 0; i < 16; i++)
            {
                int x = i & 0b11;
                int y = i >> 2;
                gfx::draw_rect(ctx->memory, 88 + (x << 3), 19 + (y << 3), 8, 8, i, true);
            }

            auto select = state.color;
            int x = select & 0b11;
            int y = select >> 2;
            gfx::draw_rect(ctx->memory, 87 + (x << 3), 18 + (y << 3), 10, 10, 1);
        }

        // 工具栏
        gfx::draw_char(ctx->memory, 0x90, 7, 85, (state.tool == EditorTool::Pencil || mouse::inside(ctx->mouse, 7, 85, 8, 8)) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x91, 15, 85, (state.tool == EditorTool::Straw || mouse::inside(ctx->mouse, 15, 85, 8, 8)) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x92, 23, 85, (state.tool == EditorTool::Barrel || mouse::inside(ctx->mouse, 23, 85, 8, 8)) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x93, 31, 85, mouse::inside(ctx->mouse, 31, 85, 8, 8) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x94, 39, 85, mouse::inside(ctx->mouse, 39, 85, 8, 8) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x95, 47, 85, mouse::inside(ctx->mouse, 47, 85, 8, 8) ? 3 : 1);
        gfx::draw_char(ctx->memory, 0x96, 55, 85, mouse::inside(ctx->mouse, 55, 85, 8, 8) ? 3 : 1);

        // 当前精灵ID
        {
            const static char hex[16] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F'};

            gfx::draw_char(ctx->memory, hex[id_y], 64, 86, 3);
            gfx::draw_char(ctx->memory, hex[id_x], 68, 86, 3);
        }

        // 精灵Flag
        if (!state.font_mode)
        {
            auto flag = 0;
            for (auto y = id_y; y < std::min(16, id_y + state.zoom); y++)
            {
                for (auto x = id_x; x < std::min(16, id_x + state.zoom); x++)
                {
                    flag |= gfx::get_flags(ctx->memory, ((y << 4) | x) & 0xFF);
                }
            }
            for (auto i = 0; i < 8; i++)
            {
                gfx::draw_rect(ctx->memory, 85 + ((i << 2) + i), 80, 4, 4, 0, true);
                if (flag & (1 << i))
                {
                    gfx::draw_rect(ctx->memory, 86 + ((i << 2) + i), 81, 2, 2, 3, true);
                }
            }
        }

        // 画笔粗细及缩放选择
        {
            gfx::draw_rect(ctx->memory, 100, 61, 17, 1, 13);
            gfx::draw_rect(ctx->memory, 100, 71, 17, 1, 13);

            for (auto i = 0; i < 3; i++)
            {
                gfx::draw_rect(ctx->memory, 99 + (i << 3), 60, 3, 3, 0, true);
                if (state.zoom == (1 << i))
                {
                    gfx::set_pixel(ctx->memory, 100 + (i << 3), 61, 3);
                }

                gfx::draw_rect(ctx->memory, 99 + (i << 3), 70, 3, 3, 0, true);
                if (state.pencil_size == 1 + i * 2)
                {
                    gfx::set_pixel(ctx->memory, 100 + (i << 3), 71, 3);
                }
            }

            gfx::draw_rect(ctx->memory, 88, 68, 7, 7, 0, true);
            gfx::draw_rect(ctx->memory,
                           91 - (state.pencil_size >> 1),
                           71 - (state.pencil_size >> 1),
                           state.pencil_size,
                           state.pencil_size,
                           1, true);
            gfx::draw_char(ctx->memory, 0x98, 87, 57, 1);
        }

        // 精灵分页
        {
            for (auto i = 0; i < 4; i++)
            {
                gfx::draw_rect(ctx->memory, 99 + i * 6, 87, 7, 6, 13);
                gfx::draw_rect(ctx->memory, 100 + i * 6, 88, 5, 4, mouse::inside(ctx->mouse, +i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            gfx::draw_rect(ctx->memory, 98 + state.page * 6, 86, 9, 8, 13);
            gfx::draw_rect(ctx->memory, 99 + state.page * 6, 87, 7, 6, 3, true);
        }

        // 精灵预览
        {
            for (auto dy = 0; dy < 32; dy++)
            {
                for (auto dx = 0; dx < 128; dx++)
                {
                    auto color = state.font_mode ? gfx::get_font(ctx->memory, dx, (state.page << 5) + dy, true) : gfx::get_sprite(ctx->memory, dx, (state.page << 5) + dy);
                    gfx::set_pixel(ctx->memory, dx, 96 + dy, color);
                }
            }

            gfx::set_clip(ctx->memory, 0, 96, 128, 32);
            // 选择的区域
            gfx::draw_rect(
                ctx->memory,
                sprite_x,
                96 + sprite_y - (state.page << 5),
                state.zoom << 3,
                state.zoom << 3,
                1);
            gfx::set_clip(ctx->memory);
        }

        gfx::draw_char(ctx->memory, 0x82, 84, 85, (state.font_mode || mouse::inside(ctx->mouse, 85, 86, 7, 7)) ? 6 : 1);
    }
}