#include "t8_core_painter.h"
#include "t8_core_memory.h"
#include "t8_utils_algo.h"

#include <algorithm>
#include <iostream>

namespace t8::core
{
    union bitfield_4
    {
        uint8_t value;
        struct
        {
            uint8_t hi : 4;
            uint8_t lo : 4;
        };
    };

    void painter_reset()
    {
        painter_reset_palette();
        painter_clip();
        painter_camera();
    }

    void painter_clip(int x, int y, int w, int h)
    {

        auto l = std::clamp(x, 0, 128);
        auto r = std::clamp(x + w, 0, 128);
        auto t = std::clamp(y, 0, 128);
        auto b = std::clamp(y + h, 0, 128);
        if (l > r || t > b)
            return;

        memory()->view_clip[0] = static_cast<uint8_t>(l);
        memory()->view_clip[1] = static_cast<uint8_t>(t);
        memory()->view_clip[2] = static_cast<uint8_t>(r - l);
        memory()->view_clip[3] = static_cast<uint8_t>(b - t);
    }

    void painter_camera(int8_t x, int8_t y)
    {
        memory()->draw_offset[0] = x;
        memory()->draw_offset[1] = y;
    }

    void painter_clear(uint8_t c)
    {
        c = (c & 0xF) | ((c & 0xF) << 4);
        std::fill(memory()->screen, memory()->screen + sizeof(memory()->screen), c);
    }

    void painter_palette_mask(uint8_t color, bool t)
    {
        if (t)
        {
            memory()->palette_mask |= (1 << color);
        }
        else
        {
            memory()->palette_mask &= ~static_cast<uint16_t>(1 < color);
        }
    }

    void painter_palette_mask(uint16_t t)
    {
        memory()->palette_mask = t;
    }

    void painter_reset_palette()
    {
        for (auto i = 0; i < 16; i++)
        {
            painter_palette(i, i);
        }
    }

    void painter_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uint32_t color = (((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff));
        memory()->palette[index & 0xF] = color;
    }

    void painter_palette(uint8_t n, uint8_t map)
    {
        if (n & 0xF0)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->palette_mapping);
        auto field = &buffer[n >> 1];
        (n & 0x1) ? (field->lo = map) : (field->hi = map);
    }

    uint8_t painter_palette(uint8_t n)
    {
        if (n & 0xF0)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->palette_mapping);
        auto field = &buffer[n >> 1];
        return (n & 0x1) ? (field->lo) : (field->hi);
    }

    void painter_pixel(int x, int y, uint8_t color)
    {
        x += memory()->draw_offset[0];
        y += memory()->draw_offset[1];
        if (x < memory()->view_clip[0] ||
            y < memory()->view_clip[1] ||
            x >= memory()->view_clip[0] + memory()->view_clip[2] ||
            y >= memory()->view_clip[1] + memory()->view_clip[3])
            return;

        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;

        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        if (!(memory()->palette_mask & (1 << color)))
        {
            (t & 1) ? (field->lo = color) : (field->hi = color);
        }
    }

    uint8_t painter_pixel(int x, int y)
    {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void painter_sprite(int x, int y, uint8_t color)
    {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        (t & 1) ? (field->lo = color) : (field->hi = color);
    }

    uint8_t painter_sprite(int x, int y)
    {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(memory()->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void painter_map(int x, int y, uint8_t n)
    {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return;
        auto i = (y * 128 + x);
        memory()->map[i] = n;
    }

    uint8_t painter_map(int x, int y)
    {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return 0;
        auto i = (y * 128 + x);
        return memory()->map[i];
    }

    void painter_flags(uint8_t n, uint8_t f)
    {
        memory()->flags[n] = f;
    }

    uint8_t painter_flags(uint8_t n)
    {
        return memory()->flags[n];
    }

    void painter_font(int x, int y, bool value, bool custom)
    {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = custom ? memory()->custom_font : memory()->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        value ? (*field |= (1 << (t & 0b111))) : (*field &= ~(1 << (t & 0b111)));
    }

    bool painter_font(int x, int y, bool custom)
    {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return false;
        auto buffer = custom ? memory()->custom_font : memory()->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        return *field & (1 << (t & 0b111));
    }

    void painter_char(uint8_t n, int x, int y, uint8_t color, bool custom)
    {
        auto sprite_x = (n & 0xF) << 3;
        auto sprite_y = (n >> 4) << 3;

        for (auto dy = 0; dy < 8; dy++)
        {
            for (auto dx = 0; dx < 8; dx++)
            {
                bool c = painter_font(sprite_x + dx, sprite_y + dy, custom);
                if (c)
                    painter_pixel(x + dx, y + dy, color);
            }
        }
    }

    void painter_line(int x0, int y0, int x1, int y1, uint8_t color)
    {
        auto dx = x1 - x0;
        auto dy = y1 - y0;
        auto fx = dx == 0 ? 0 : (dx > 0 ? 1 : -1);
        auto fy = dy == 0 ? 0 : (dy > 0 ? 1 : -1);
        auto x = x0;
        auto y = y0;
        dx = std::abs(dx);
        dy = std::abs(dy);
        auto delta = std::min(dx, dy) * 2;
        auto error = 0;

        if (dx > dy) {
            for (; x != x1; x += fx) {
                painter_pixel(x, y, color);
                if ((error += delta) > dx) {
                    y += fy;
                    error -= dx * 2;
                }
            }
        } else {
            for (; y != y1; y += fy) {
                painter_pixel(x, y, color);
                if ((error += delta) > dy) {
                    x += fx;
                    error -= dy * 2;
                }
            }
        }
    }

    void painter_circle(int xc, int yc, int r, uint8_t color, bool fill)
    {
        const auto put = [&](int xc, int yc, int x, int y)
        {
            painter_pixel(xc + x, yc + y, color);
            painter_pixel(xc - x, yc + y, color);
            painter_pixel(xc + x, yc - y, color);
            painter_pixel(xc - x, yc - y, color);
            painter_pixel(xc + y, yc + x, color);
            painter_pixel(xc - y, yc + x, color);
            painter_pixel(xc + y, yc - x, color);
            painter_pixel(xc - y, yc - x, color);
        };

        int x = 0, y = r;
        int d = 3 - 2 * r;

        while (y >= x)
        {
            if (d > 0)
            {
                y--;
                d = d + 4 * (x - y) + 10;
            }
            else
                d = d + 4 * x + 6;
            x++;
            if (fill)
            {
                for (auto i = 0; i < r - x; i++)
                {
                    put(xc, yc, x, y + i);
                }
            }
            else
            {
                put(xc, yc, x, y);
            }
        }
    }

    void painter_rect(int x, int y, int w, int h, uint8_t color, bool fill)
    {
        for (auto dx = 0; dx < w; dx += 1)
        {
            painter_pixel(x + dx, y, color);
            painter_pixel(x + dx, y + h - 1, color);
        }
        for (auto dy = 0; dy < h; dy += 1)
        {
            painter_pixel(x, y + dy, color);
            painter_pixel(x + w - 1, y + dy, color);
        }

        if (fill)
        {
            auto l = std::clamp(x + 1, 0, 128);
            auto r = std::clamp(x + w - 1, 0, 128);
            auto t = std::clamp(y + 1, 0, 128);
            auto b = std::clamp(y + h - 1, 0, 128);

            for (auto y = t; y < b; y += 1)
            {
                for (auto x = l; x < r; x += 1)
                {
                    painter_pixel(x, y, color);
                }
            }
        }
    }

}
