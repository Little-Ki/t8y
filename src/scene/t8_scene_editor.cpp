#include "t8_scene_editor.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keyboard.h"
#include "t8_input_mouse.h"
#include "t8_utils_timer.h"

#include "t8_text_editor.hpp"

#include <functional>

using namespace t8::core;
using namespace t8::utils;
using namespace t8::input;

namespace t8::scene
{

    enum class EditorPage
    {
        Script,
        Sprite,
        Map
    };

    enum class EditorTool
    {
        Pencil,
        Straw,
        Barrel
    };

    struct SheetDrawTarget
    {
        std::function<void(int, int, uint8_t)> setter;
        std::function<uint8_t(int, int)> getter;
    };

    struct ScriptEditorState
    {
        int scroll_x = 0;
        int scroll_y = 0;
        bool dragging = false;
        t8::text_editor::TextEditor editor;
    };

    struct SheetEditorState
    {
        EditorTool tool = EditorTool::Pencil;
        uint8_t sprite_id = 0;
        uint8_t color = 1;
        uint8_t page = 0;
        uint8_t zoom = 1;
        uint8_t pencil_size = 1;
        bool font_mode = false;
    };

    struct MapEditorState
    {
        uint8_t sprite_id = 0;
        uint8_t zoom = 1;
        uint8_t page = 0;
        int x = 0;
        int y = 11;
    };

    EditorPage page = EditorPage::Script;
    ScriptEditorState script_state;
    SheetEditorState sheet_state;
    MapEditorState map_state;

    void editor_draw_tab()
    {
        painter_rect(0, 0, 128, 11, 13, true);
        painter_char(0x80, 1, 1, (page == EditorPage::Script || mouse_inside(2, 2, 7, 7)) ? 7 : 1);
        painter_char(0x81, 10, 1, (page == EditorPage::Sprite || mouse_inside(11, 2, 7, 7)) ? 7 : 1);
        painter_char(0x83, 19, 1, (page == EditorPage::Map || mouse_inside(20, 2, 7, 7)) ? 7 : 1);
    }

    void editor_update_tab()
    {
        if (mouse_clicked(1, 1, 8, 8))
        {
            ctx_signals().push({SIGNAL_START_INPUT});
            page = EditorPage::Script;
        }
        if (mouse_clicked(10, 1, 8, 8))
        {
            ctx_signals().push({SIGNAL_STOP_INPUT});
            page = EditorPage::Sprite;
        }
        if (mouse_clicked(19, 1, 8, 8))
        {
            ctx_signals().push({SIGNAL_STOP_INPUT});
            page = EditorPage::Map;
        }
    }

    void editor_spray_sheet(
        int area_x, int area_y, int area_size,
        int x, int y,
        uint8_t replace, uint8_t color,
        const SheetDrawTarget &target)
    {
        struct Point
        {
            int x, y;
        };

        std::stack<Point> points;

        points.push({x, y});

        while (!points.empty())
        {
            const auto p = points.top();
            points.pop();

            if (p.x < 0 || p.x >= 128 || p.y < 0 || p.y >= 128)
                continue;

            if (p.x < area_x || p.x >= area_x + area_size || p.y < area_y || p.y >= area_y + area_size)
                continue;

            const auto it = target.getter(p.x, p.y);

            if (it == color)
                continue;

            if (it == replace)
            {
                target.setter(p.x, p.y, color);
                points.push({p.x - 1, p.y});
                points.push({p.x + 1, p.y});
                points.push({p.x, p.y - 1});
                points.push({p.x, p.y + 1});
            }
        }
    }

    void editor_draw_sheet_pixel(
        int area_x, int area_y, int area_size,
        int x, int y, int pixel_size,
        uint8_t color,
        const SheetDrawTarget &target)
    {
        auto l = std::max(x - (pixel_size >> 1), area_x);
        auto t = std::max(y - (pixel_size >> 1), area_y);
        auto r = std::min(x - (pixel_size >> 1) + pixel_size, area_x + area_size);
        auto b = std::min(y - (pixel_size >> 1) + pixel_size, area_y + area_size);

        for (auto y = t; y < b; y++)
        {
            for (auto x = l; x < r; x++)
            {
                target.setter(x, y, color);
            }
        }
    }

    void editor_erase_sheet(
        int area_x, int area_y, int size,
        const SheetDrawTarget &target)
    {
        for (auto y = area_y; y < area_y + size; y++)
        {
            for (auto x = area_x; x < area_x + size; x++)
            {
                target.setter(x, y, 0);
            }
        }
    }

    void editor_rotate_sheet(
        int x, int y, int size,
        const SheetDrawTarget &target)
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
                auto prev = target.getter(x + dx, y + dy);
                for (auto i = 0; i < 4; i++)
                {
                    next(tx, ty);
                    auto it = target.getter(x + tx, y + ty);
                    target.setter(x + tx, y + ty, prev);
                    prev = it;
                }
            }
        }
    }

    void editor_flip_sheet(
        int area_x, int area_y, int size,
        bool vertical,
        const SheetDrawTarget &target)
    {
        if (vertical)
        {
            for (auto dy = 0; dy < (size >> 1); dy++)
            {
                for (auto dx = 0; dx < size; dx++)
                {
                    auto that = target.getter(area_x + dx, area_y + size - dy - 1);
                    auto it = target.getter(area_x + dx, area_y + dy);
                    target.setter(area_x + dx, area_y + size - dy - 1, it);
                    target.setter(area_x + dx, area_y + dy, that);
                }
            }
        }
        else
        {
            for (auto dy = 0; dy < size; dy++)
            {
                for (auto dx = 0; dx < (size >> 1); dx++)
                {
                    auto that = target.getter(area_x + size - dx - 1, area_y + dy);
                    auto it = target.getter(area_x + dx, area_y + dy);
                    target.setter(area_x + size - dx - 1, area_y + dy, it);
                    target.setter(area_x + dx, area_y + dy, that);
                }
            }
        }
    }

    void editor_draw_sheet_edit()
    {
        const auto sprite_id = sheet_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x * 8;
        const auto sprite_y = id_y * 8;
        auto pixel_size = 8 / sheet_state.zoom;
        auto range = sheet_state.zoom * 8;

        painter_rect(0, 11, 128, 84, 14, true);

        // 画板
        painter_rect(7, 18, 66, 66, 0);
        for (auto dy = 0; dy < range; dy++)
        {
            for (auto dx = 0; dx < range; dx++)
            {
                auto color = sheet_state.font_mode ? painter_font(sprite_x + dx, sprite_y + dy, true) : painter_sprite(sprite_x + dx, sprite_y + dy);
                painter_rect(8 + dx * pixel_size, 19 + dy * pixel_size, pixel_size, pixel_size, color, true);
            }
        }

        //  调色板
        {
            painter_rect(87, 18, 34, 34, 0);
            for (auto i = 0; i < 16; i++)
            {
                int x = i & 0b11;
                int y = i >> 2;
                painter_rect(88 + (x << 3), 19 + (y << 3), 8, 8, i, true);
            }

            auto select = sheet_state.color;
            int x = select & 0b11;
            int y = select >> 2;
            painter_rect(87 + (x << 3), 18 + (y << 3), 10, 10, 1);
        }

        // 工具栏
        painter_char(0x90, 7, 85, (sheet_state.tool == EditorTool::Pencil || mouse_inside(7, 85, 8, 8)) ? 3 : 1);
        painter_char(0x91, 15, 85, (sheet_state.tool == EditorTool::Straw || mouse_inside(15, 85, 8, 8)) ? 3 : 1);
        painter_char(0x92, 23, 85, (sheet_state.tool == EditorTool::Barrel || mouse_inside(23, 85, 8, 8)) ? 3 : 1);
        painter_char(0x93, 31, 85, mouse_inside(31, 85, 8, 8) ? 3 : 1);
        painter_char(0x94, 39, 85, mouse_inside(39, 85, 8, 8) ? 3 : 1);
        painter_char(0x95, 47, 85, mouse_inside(47, 85, 8, 8) ? 3 : 1);
        painter_char(0x96, 55, 85, mouse_inside(55, 85, 8, 8) ? 3 : 1);

        // 当前精灵ID
        {
            const static char hex[16] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F'};

            painter_char(hex[id_y], 64, 86, 3);
            painter_char(hex[id_x], 68, 86, 3);
        }

        // 精灵Flag
        if (!sheet_state.font_mode)
        {
            auto flag = 0;
            for (auto y = id_y; y < std::min(16, id_y + sheet_state.zoom); y++)
            {
                for (auto x = id_x; x < std::min(16, id_x + sheet_state.zoom); x++)
                {
                    flag |= painter_flags(((y << 4) | x) & 0xFF);
                }
            }
            for (auto i = 0; i < 8; i++)
            {
                painter_rect(85 + ((i << 2) + i), 80, 4, 4, 0, true);
                if (flag & (1 << i))
                {
                    painter_rect(86 + ((i << 2) + i), 81, 2, 2, 3, true);
                }
            }
        }

        // 画笔粗细及缩放选择
        {
            painter_rect(100, 61, 17, 1, 13);
            painter_rect(100, 71, 17, 1, 13);

            for (auto i = 0; i < 3; i++)
            {
                painter_rect(99 + (i << 3), 60, 3, 3, 0, true);
                if (sheet_state.zoom == (1 << i))
                {
                    painter_pixel(100 + (i << 3), 61, 3);
                }

                painter_rect(99 + (i << 3), 70, 3, 3, 0, true);
                if (sheet_state.pencil_size == 1 + i * 2)
                {
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

        // 精灵分页
        {
            for (auto i = 0; i < 4; i++)
            {
                painter_rect(99 + i * 6, 87, 7, 6, 13);
                painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            painter_rect(98 + sheet_state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + sheet_state.page * 6, 87, 7, 6, 3, true);
        }

        // 精灵预览
        {
            for (auto dy = 0; dy < 32; dy++)
            {
                for (auto dx = 0; dx < 128; dx++)
                {
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
        painter_char(0x82, 84, 85, (sheet_state.font_mode || mouse_inside(85, 86, 7, 7)) ? 6 : 1);
    }

    void editor_update_sheet_edit()
    {
        const auto sprite_id = sheet_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x << 3;
        const auto sprite_y = id_y << 3;
        const auto area_size = sheet_state.zoom << 3;

        const SheetDrawTarget draw_font = {
            [](int x, int y, uint8_t c)
            { painter_font(x, y, c, true); },
            [](int x, int y)
            { return painter_font(x, y, true); }};
        const SheetDrawTarget draw_sprite = {
            [](int x, int y, uint8_t c)
            { painter_sprite(x, y, c); },
            [](int x, int y)
            { return painter_sprite(x, y); }};

        const SheetDrawTarget draw_target = sheet_state.font_mode ? draw_font : draw_sprite;

        // 工具操作
        {
            if (mouse_clicked(8, 86, 7, 7))
            {
                sheet_state.tool = EditorTool::Pencil;
            }
            if (mouse_clicked(16, 86, 7, 7))
            {
                sheet_state.tool = EditorTool::Straw;
            }
            if (mouse_clicked(24, 86, 7, 7))
            {
                sheet_state.tool = EditorTool::Barrel;
            }
            if (mouse_clicked(32, 86, 7, 7))
            {
                editor_flip_sheet(sprite_x, sprite_y, area_size, false, draw_target);
            }
            if (mouse_clicked(40, 86, 7, 7))
            {
                editor_flip_sheet(sprite_x, sprite_y, area_size, true, draw_target);
            }
            if (mouse_clicked(48, 86, 7, 7))
            {
                editor_rotate_sheet(sprite_x, sprite_y, area_size, draw_target);
            }
            if (mouse_clicked(56, 86, 7, 7))
            {
                editor_erase_sheet(sprite_x, sprite_y, area_size, draw_target);
            }
        }

        // 字体模式
        if (mouse_clicked(85, 86, 7, 7))
        {
            sheet_state.font_mode = !sheet_state.font_mode;
        }

        // 精灵页面
        for (auto i = 0; i < 4; i++)
        {
            if (mouse_clicked(100 + i * 6, 88, 5, 4))
            {
                sheet_state.page = i;
            }
        }

        // 调色板
        if (mouse_dragging(88, 19, 32, 32))
        {
            int x = (mouse_x() - 88) >> 3;
            int y = (mouse_y() - 19) >> 3;
            sheet_state.color = (y << 2) | x;
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32))
        {
            auto local_x = mouse_x();
            auto local_y = mouse_y() - 96;
            local_x >>= 3;
            local_y >>= 3;
            auto next_id = ((local_y << 4) | (local_x & 0xF)) & 0xFF;
            next_id += (sheet_state.page << 6);
            sheet_state.sprite_id = next_id;
        }
        else if (mouse_inside(0, 96, 128, 32))
        {
            if (mouse_z() < 0)
            {
                sheet_state.page = std::clamp(sheet_state.page + 1, 0, 3);
            }
            if (mouse_z() > 0)
            {
                sheet_state.page = std::clamp(sheet_state.page - 1, 0, 3);
            }
        }

        // 右侧滑块
        for (auto i = 0; i < 3; i++)
        {
            if (mouse_clicked(98 + (i << 3), 59, 5, 5))
            {
                sheet_state.zoom = (1 << i);
            }
            if (mouse_clicked(98 + (i << 3), 69, 5, 5))
            {
                sheet_state.pencil_size = 1 + i * 2;
            }
        }

        // 设置精灵Flag
        if (!sheet_state.font_mode)
        {
            auto old = 0;
            for (auto y = id_y; y < ((id_y + sheet_state.zoom) & 0xF); y++)
            {
                for (auto x = id_x; x < ((id_x + sheet_state.zoom) & 0xF); x++)
                {
                    old |= painter_flags(((y << 4) | x) & 0xFF);
                }
            }

            for (auto i = 0; i < 8; i++)
            {
                if (mouse_clicked(85 + ((i << 2) + i), 81, 4, 4))
                {
                    for (auto y = id_y; y < ((id_y + sheet_state.zoom) & 0xF); y++)
                    {
                        for (auto x = id_x; x < ((id_x + sheet_state.zoom) & 0xF); x++)
                        {
                            auto flag = painter_flags(((y << 4) | x) & 0xFF);
                            if (old & (1 << i))
                            {
                                flag &= ~static_cast<uint8_t>(1 << i);
                            }
                            else
                            {
                                flag |= static_cast<uint8_t>(1 << i);
                            }
                            painter_flags(((y << 4) | x) & 0xFF, flag);
                        }
                    }
                }
            }
        }

        // 绘制操作
        {

            auto local_x = mouse_x() - 8;
            auto local_y = mouse_y() - 19;

            local_x /= 8 >> (sheet_state.zoom >> 1);
            local_y /= 8 >> (sheet_state.zoom >> 1);

            if (sheet_state.tool == EditorTool::Straw)
            {
                if (mouse_clicked(8, 19, 64, 64))
                {
                    sheet_state.color = draw_target.getter(sprite_x + local_x, sprite_y + local_y);
                    sheet_state.tool = EditorTool::Pencil;
                }
            }

            if (sheet_state.tool == EditorTool::Barrel)
            {
                if (mouse_clicked(8, 19, 64, 64))
                {
                    auto replace = draw_target.getter(sprite_x + local_x, sprite_y + local_y);

                    editor_spray_sheet(
                        sprite_x, sprite_y,
                        area_size,
                        sprite_x + local_x, sprite_y + local_y,
                        replace,
                        sheet_state.color,
                        draw_target);

                    sheet_state.tool = EditorTool::Pencil;
                }
            }
            if (sheet_state.tool == EditorTool::Pencil)
            {
                if (mouse_dragging(8, 19, 64, 64))
                {
                    editor_draw_sheet_pixel(
                        sprite_x, sprite_y, area_size,
                        sprite_x + local_x, sprite_y + local_y,
                        sheet_state.pencil_size, sheet_state.color,
                        draw_target);
                }
            }
            if (mouse_inside(8, 19, 64, 64))
            {
                if (mouse_z() < 0)
                {
                    sheet_state.zoom = std::clamp(sheet_state.zoom << 1, 1, 4);
                }
                if (mouse_z() > 0)
                {
                    sheet_state.zoom = std::clamp(sheet_state.zoom >> 1, 1, 4);
                }
            }
        }
    }

    void editor_draw_map_edit()
    {
        const auto sprite_id = map_state.sprite_id;
        const auto id_x = (sprite_id & 0xF);
        const auto id_y = (sprite_id >> 4) & 0xF;
        const auto sprite_x = id_x << 3;
        const auto sprite_y = id_y << 3;

        painter_rect(0, 11, 128, 84, 10, true);

        // 网格背景
        for (auto y = 11; y < 85; y++)
        {
            for (auto x = 0; x < 128; x++)
            {
                if ((x + y) & 1)
                {
                    painter_pixel(x, y, 8);
                }
                else
                {
                    painter_pixel(x, y, 0);
                }
            }
        }

        painter_rect(0, 85, 128, 10, 14, true);

        {
            for (auto i = 0; i < 4; i++)
            {
                painter_rect(99 + i * 6, 87, 7, 6, 13);
                painter_rect(100 + i * 6, 88, 5, 4, mouse_inside(100 + i * 6, 88, 5, 4) ? 14 : 15, true);
            }

            painter_rect(98 + map_state.page * 6, 86, 9, 8, 13);
            painter_rect(99 + map_state.page * 6, 87, 7, 6, 3, true);
        }

        // 下方精灵图预览及选择
        {

            for (auto dy = 0; dy < 32; dy++)
            {
                for (auto dx = 0; dx < 128; dx++)
                {
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

            for (auto y = t; y < b; y++)
            {
                for (auto x = l; x < r; x++)
                {
                    auto id = painter_map(x, y);
                    auto spX = (id & 0xF) << 3;
                    auto spY = ((id >> 4) & 0xF) << 3;

                    for (auto inY = 0; inY < chunk_size; inY++)
                    {
                        for (auto inX = 0; inX < chunk_size; inX++)
                        {
                            auto color = id == 0 ? 0 : painter_sprite(spX + inX * step, spY + inY * step);
                            painter_pixel(
                                map_state.x + x * chunk_size + inX,
                                map_state.y + y * chunk_size + inY,
                                color);
                        }
                    }
                }
            }

            if (mouse_inside(0, 11, 128, 74))
            {
                auto innerX = mouse_x() - map_state.x;
                auto innerY = mouse_y() - map_state.y;
                innerX /= chunk_size;
                innerY /= chunk_size;

                if (innerX >= 0 && innerX < 128 && innerY >= 0 && innerY < 128)
                {
                    painter_rect(map_state.x + innerX * chunk_size - 1, map_state.y + innerY * chunk_size - 1, chunk_size + 2, chunk_size + 2, 1);
                }
            }

            painter_clip();
        }
    }

    void editor_update_map_edit()
    {

        // 选择分组
        for (auto i = 0; i < 4; i++)
        {
            if (mouse_clicked(100 + i * 6, 88, 5, 4))
            {
                map_state.page = i;
            }
        }

        // 选择精灵
        if (mouse_dragging(0, 96, 128, 32))
        {
            auto local_x = mouse_x();
            auto local_y = mouse_y() - 96;
            local_x >>= 3;
            local_y >>= 3;
            auto next_id = ((local_y << 4) | (local_x & 0xF)) & 0xFF;
            next_id += (map_state.page << 6);
            map_state.sprite_id = next_id;
        }
        else if (mouse_inside(0, 96, 128, 32))
        {
            if (mouse_z() < 0)
            {
                map_state.page = std::clamp(map_state.page + 1, 0, 3);
            }
            if (mouse_z() > 0)
            {
                map_state.page = std::clamp(map_state.page - 1, 0, 3);
            }
        }

        // 绘制地图
        if (mouse_dragging(0, 11, 128, 74))
        {
            auto local_x = mouse_x() - map_state.x;
            auto local_y = mouse_y() - map_state.y;
            local_x /= 8 >> (map_state.zoom >> 1);
            local_y /= 8 >> (map_state.zoom >> 1);
            painter_map(local_x, local_y, map_state.sprite_id);
        }
        else if (mouse_dragging(0, 11, 128, 74, 2))
        {
            map_state.x += mouse_dx();
            map_state.y += mouse_dy();
        }
        else if (mouse_inside(0, 11, 128, 74))
        {
            const auto z = mouse_z();
            if (z != 0)
            {
                const auto next = std::clamp(z < 0 ? map_state.zoom << 1 : map_state.zoom >> 1, 1, 4);
                const auto scale = z < 0 ? 0.5f : 2.0f;
                if (next != map_state.zoom)
                {
                    map_state.x = 64 + static_cast<int>((map_state.x - 64) * scale);
                    map_state.y = 48 + static_cast<int>((map_state.y - 48) * scale);
                    map_state.zoom = next;
                }
            }
        }
    }

    size_t editor_script_edit_visual_column(size_t line, size_t cursor)
    {
        if (line < 0 || line >= script_state.editor.line_count())
            return 0;

        const auto line_start = script_state.editor.line_start(line);
        const auto line_size = script_state.editor.line_size(line);
        const auto max_column = std::min(line_size, cursor - line_start);
        size_t result = 0;

        for (size_t col = 0; col < max_column; col += 1)
        {
            const auto ch = script_state.editor.get_char(line, col);
            const auto w = ch == '\t' ? 8 : 4;
            result += w;
        }

        return result;
    }

    void editor_script_edit_cursor_lifting(size_t next_line, size_t visual_column, bool collapse = true)
    {
        if (next_line < 0 || next_line >= script_state.editor.line_count())
            return;

        const auto line_start = script_state.editor.line_start(next_line);
        const auto line_size = script_state.editor.line_size(next_line);
        size_t temp = 0;
        size_t col = 0;

        for (; col < line_size; col += 1)
        {
            const auto ch = script_state.editor.get_char(next_line, col);
            const auto w = ch == '\t' ? 8 : 4;
            if (temp >= visual_column)
            {
                break;
            }
            temp += w;
        }

        const size_t shift = (col == line_size - 1 && line_start + col == script_state.editor.size()) ? 1 : 0;

        if (collapse)
            script_state.editor.collapse(line_start + col - shift);
        else
            script_state.editor.set_cursor(line_start + col - shift);
    }

    void editor_draw_script_edit()
    {
        painter_rect(0, 11, 8, 117, 14, true);
        painter_rect(8, 11, 120, 117, 15, true);

        const auto sel_start = script_state.editor.sel_start();
        const auto sel_end = script_state.editor.sel_end();
        const auto cursor = script_state.editor.cursor();

        painter_camera(8, 11);
        painter_clip(8, 11, 120, 117);

        auto line_count = script_state.editor.line_count();
        auto above = script_state.scroll_y;
        auto below = std::min(line_count, static_cast<size_t>(above) + 16);

        auto x = -script_state.scroll_x * 4;
        auto y = -script_state.scroll_y * 8 + above * 8;

        for (auto line = above; line < below; line += 1)
        {
            auto line_size = script_state.editor.line_size(line);
            auto line_start = script_state.editor.line_start(line);

            for (auto col = 0; col < line_size; col += 1)
            {
                const auto ch = script_state.editor.get_char(line, col);
                const auto current_index = line_start + col;
                if ((timer_ticks() >> 5) % 2 && current_index == cursor)
                    painter_rect(x, y, 1, 7, 3, true);

                if (current_index >= sel_start && current_index < sel_end && ch != '\n')
                    painter_rect(x, y, ch == '\t' ? 8 : 4, 7, 3, true);

                if (ch == '\t')
                {
                    x += 8;
                }
                else if (ch != '\n')
                {
                    painter_char(ch, x, y, 1);
                    x += 4;
                }
            }

            if ((timer_ticks() >> 5) % 2 && line == below - 1 && line_start + line_size == cursor)
                painter_rect(x, y, 1, 7, 3, true);

            x = -script_state.scroll_x * 4;
            y += 8;
        }

        painter_camera();
        painter_clip();
    }

    void editor_update_script_edit()
    {
        auto auto_scroll = true;
        auto cursor = script_state.editor.cursor();

        if (!context()->inputs.empty())
        {
            const auto text = context()->inputs.back();
            context()->inputs.pop();

            for (const auto &ch : text)
                if (ch & 0x80)
                    return;

            script_state.editor.insert(text);
        }
        else
        {
            if (keyboard_triggered(SCANCODE_ENTER) ||
                keyboard_triggered(SCANCODE_RETURN))
            {
                script_state.editor.insert('\n');
            }
            else if (keyboard_triggered(SCANCODE_BACKSPACE))
            {
                script_state.editor.erase_before();
            }
            else if (keyboard_triggered(SCANCODE_DELETE))
            {
                script_state.editor.erase_after();
            }
            else if (keyboard_triggered(SCANCODE_UP))
            {
                const auto cursor = script_state.editor.cursor();
                const auto line = script_state.editor.line_index(cursor);
                if (line > 0)
                {
                    const auto col = editor_script_edit_visual_column(line, cursor);
                    editor_script_edit_cursor_lifting(line - 1, col);
                }
            }
            else if (keyboard_triggered(SCANCODE_DOWN))
            {
                const auto cursor = script_state.editor.cursor();
                const auto line = script_state.editor.line_index(cursor);
                if (line < script_state.editor.line_count() - 1)
                {
                    const auto col = editor_script_edit_visual_column(line, cursor);
                    editor_script_edit_cursor_lifting(line + 1, col);
                }
            }
            else if (keyboard_triggered(SCANCODE_LEFT))
            {
                if (cursor > 0)
                {
                    script_state.editor.collapse(cursor - 1);
                }
            }
            else if (keyboard_triggered(SCANCODE_RIGHT))
            {
                script_state.editor.collapse(cursor + 1);
            }
            else if (keyboard_pressed(SCANCODE_A))
            {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl())
                {
                    script_state.editor.select_all();
                }
            }
            else if (keyboard_pressed(SCANCODE_Z))
            {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl())
                {
                    script_state.editor.undo();
                }
            }
            else if (keyboard_pressed(SCANCODE_Y))
            {
                if (!keyboard_alt() && !keyboard_shift() && keyboard_ctrl())
                {
                    script_state.editor.redo();
                }
            }
            else if (keyboard_pressed(SCANCODE_TAB))
            {
                if (!keyboard_alt() && !keyboard_shift() && !keyboard_ctrl())
                {
                    script_state.editor.insert("\t");
                }
            }
            else
            {
                auto_scroll = false;
            }
        }

        if (mouse_inside(8, 11, 120, 128) || script_state.dragging)
        {
            const auto local_x = mouse_x() - 8 + script_state.scroll_x * 4;
            const auto local_y = mouse_y() - 11 + script_state.scroll_y * 8;

            auto line = local_y / 8;

            if (line >= static_cast<int>(script_state.editor.line_count()))
                line = static_cast<int>(script_state.editor.line_count()) - 1;

            if (line < 0)
                line = 0;

            const auto line_start = script_state.editor.line_start(line);
            const auto line_size = script_state.editor.line_size(line);

            auto cursor_index = line_start;
            int x = 0;

            for (size_t col = 0; col < line_size; col += 1)
            {
                const auto ch = script_state.editor.get_char(line, col);
                const auto w = ch == '\t' ? 8 : 4;
                int middle = x + w / 2;

                if (local_x >= x && local_x < x + w / 2)
                {
                    cursor_index = line_start + col;
                }

                if (local_x >= x + w / 2 && local_x < x + w)
                {
                    cursor_index = line_start + col + 1;
                }

                x += w;
            }

            if (local_x > x)
            {
                if (line == static_cast<int>(script_state.editor.line_count()) - 1)
                {
                    cursor_index = script_state.editor.size();
                }
                else
                {
                    cursor_index = line_start + line_size - 1;
                }
            }

            if (mouse_pressed(1))
            {
                script_state.editor.collapse(cursor_index);
                auto_scroll = true;
                script_state.dragging = true;
            }
            else if (mouse_down(1))
            {
                script_state.editor.set_cursor(cursor_index);
                auto_scroll = true;
            }
            else
            {
                script_state.dragging = false;
            }
        }

        if (auto_scroll)
        {
            const auto cursor = script_state.editor.cursor();
            const auto line = script_state.editor.line_index(cursor);
            const auto line_start = script_state.editor.line_start(line);
            const auto line_size = script_state.editor.line_size(line);
            const auto column = cursor - line_start;
            const auto delta_y = static_cast<int>(line) - script_state.scroll_y;
            const auto delta_x = static_cast<int>(column) - script_state.scroll_x;
            if (delta_y > 12)
            {
                script_state.scroll_y = static_cast<int>(line) - 13;
            }
            else if (delta_y < 1)
            {
                script_state.scroll_y = static_cast<int>(line);
            }

            if (delta_x > 29)
            {
                script_state.scroll_x = static_cast<int>(column) - 29;
            }
            else if (delta_x < 0)
            {
                script_state.scroll_x = static_cast<int>(column);
            }
        }
    }

    void editor_update()
    {
        if (keyboard_pressed(41))
        {
            ctx_signals().push({SIGNAL_SWAP_CONSOLE});
            return;
        }
        editor_update_tab();
        if (page == EditorPage::Script)
        {
            editor_update_script_edit();
        }
        if (page == EditorPage::Sprite)
        {
            editor_update_sheet_edit();
        }
        if (page == EditorPage::Map)
        {
            editor_update_map_edit();
        }
    }

    void editor_draw()
    {
        painter_clear(0);
        editor_draw_tab();

        if (page == EditorPage::Script)
        {
            editor_draw_script_edit();
        }
        if (page == EditorPage::Sprite)
        {
            editor_draw_sheet_edit();
        }
        if (page == EditorPage::Map)
        {
            editor_draw_map_edit();
        }
    }

    void editor_enter()
    {
        painter_reset();

        script_state.editor.reset("function init()\n\tline1\n\tline2\nend");
        // script_state.editor.reset(get_script());

        if (page == EditorPage::Script)
        {
            ctx_signals().push({SIGNAL_START_INPUT});
        }

        timer_reset();
    }

    void editor_leave()
    {
        ctx_script() = script_state.editor.to_string();
        ctx_signals().push({SIGNAL_STOP_INPUT});
    }
}