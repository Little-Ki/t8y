#include "t8_graphic.h"
#include "t8_memory.h"
#include "t8_types.h"
#include "t8_utils.h"

#include <algorithm>

namespace t8 {
    GraphicState state;

    void graphic_reset() {
        graphic_reset_palette();
        graphic_clip();
        graphic_camera();
    }

    void graphic_backup_palette() {
        std::copy(mem()->palette, mem()->palette + 0x8, state.prev_palette);
    }

    void graphic_restore_palette() {
        std::copy(state.prev_palette, state.prev_palette + 0x8, mem()->palette);
    }

    void graphic_clip(int x, int y, int w, int h) {

        auto l = std::clamp(x, 0, 128);
        auto r = std::clamp(x + w, 0, 128);
        auto t = std::clamp(y, 0, 128);
        auto b = std::clamp(y + h, 0, 128);
        if (l > r || t > b)
            return;

        mem()->view_clip[0] = static_cast<uint8_t>(l);
        mem()->view_clip[1] = static_cast<uint8_t>(t);
        mem()->view_clip[2] = static_cast<uint8_t>(r - l);
        mem()->view_clip[3] = static_cast<uint8_t>(b - t);
    }

    void graphic_camera(int8_t x, int8_t y) {
        mem()->draw_offset[0] = x;
        mem()->draw_offset[1] = y;
    }

    void graphic_clear(uint8_t c) {
        c = (c & 0xF) | ((c & 0xF) << 4);
        std::fill(mem()->screen, mem()->screen + sizeof(mem()->screen), c);
    }

    void graphic_opacity(uint8_t color, bool t) {
        if (t) {
            mem()->opacity |= (1 < color);
        } else {
            mem()->opacity &= ~static_cast<uint16_t>(1 < color);
        }
    }

    void graphic_opacity(uint16_t t) {
        mem()->opacity = t;
    }

    void graphic_reset_palette() {
        for (auto i = 0; i < 16; i++) {
            graphic_palette(i, i);
        }
    }

    void graphic_palette(uint8_t n, uint8_t map) {
        if (n & 0xF0)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->palette);
        auto field = &buffer[n >> 1];
        (n & 0x1) ? (field->lo = map) : (field->hi = map);
    }

    uint8_t graphic_palette(uint8_t n) {
        if (n & 0xF0)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->palette);
        auto field = &buffer[n >> 1];
        return (n & 0x1) ? (field->lo) : (field->hi);
    }

    void graphic_pixel(int x, int y, uint8_t color) {
        x += mem()->draw_offset[0];
        y += mem()->draw_offset[1];
        if (x < mem()->view_clip[0] ||
            y < mem()->view_clip[1] ||
            x >= mem()->view_clip[0] + mem()->view_clip[2] ||
            y >= mem()->view_clip[1] + mem()->view_clip[3])
            return;

        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;

        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        (t & 1) ? (field->lo = color) : (field->hi = color);
    }

    uint8_t graphic_pixel(int x, int y) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void graphic_sprite(int x, int y, uint8_t color) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        (t & 1) ? (field->lo = color) : (field->hi = color);
    }

    uint8_t graphic_sprite(int x, int y) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(mem()->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void graphic_map(int x, int y, uint8_t n) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return;
        auto i = (y * 128 + x);
        mem()->map[i] = n;
    }

    uint8_t graphic_map(int x, int y) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return 0;
        auto i = (y * 128 + x);
        return mem()->map[i];
    }

    void graphic_flags(uint8_t n, uint8_t f) {
        mem()->flags[n] = f;
    }

    uint8_t graphic_flags(uint8_t n) {
        return mem()->flags[n];
    }

    void graphic_font(int x, int y, bool value, bool custom) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = custom ? mem()->custom_font : mem()->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        value ? (*field |= (1 << (t & 0b111))) : (*field &= ~(1 << (t & 0b111)));
    }

    bool graphic_font(int x, int y, bool custom) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return false;
        auto buffer = custom ? mem()->custom_font : mem()->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        return *field & (1 << (t & 0b111));
    }

    void graphic_char(uint8_t n, int x, int y, uint8_t color, bool custom) {
        auto baseX = (n & 0xF) << 3;
        auto baseY = (n >> 4) << 3;

        for (auto dy = 0; dy < 8; dy++) {
            for (auto dx = 0; dx < 8; dx++) {
                bool c = graphic_font(baseX + dx, baseY + dy, custom);
                if (c)
                    graphic_pixel(x + dx, y + dy, color);
            }
        }
    }

    void graphic_line(int x0, int y0, int x1, int y1, uint8_t color) {
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
                graphic_pixel(x, y, color);
                if ((error += delta) > dx) {
                    y += fy;
                    error -= dx * 2;
                }
            }
        } else {
            for (; y != y1; y += fy) {
                graphic_pixel(x, y, color);
                if ((error += delta) > dy) {
                    x += fx;
                    error -= dy * 2;
                }
            }
        }
    }

    void graphic_circle(int xc, int yc, int r, uint8_t color, bool fill) {
        const auto put = [&](int xc, int yc, int x, int y) {
            graphic_pixel(xc + x, yc + y, color);
            graphic_pixel(xc - x, yc + y, color);
            graphic_pixel(xc + x, yc - y, color);
            graphic_pixel(xc - x, yc - y, color);
            graphic_pixel(xc + y, yc + x, color);
            graphic_pixel(xc - y, yc + x, color);
            graphic_pixel(xc + y, yc - x, color);
            graphic_pixel(xc - y, yc - x, color);
        };

        int x = 0, y = r;
        int d = 3 - 2 * r;

        while (y >= x) {
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else
                d = d + 4 * x + 6;
            x++;
            if (fill) {
                for (auto i = 0; i < r - x; i++) {
                    put(xc, yc, x, y + i);
                }
            } else {
                put(xc, yc, x, y);
            }
        }
    }

    void graphic_rect(int x, int y, int w, int h, uint8_t color, bool fill) {
        for (auto dx = 0; dx < w; dx += 1) {
            graphic_pixel(x + dx, y, color);
            graphic_pixel(x + dx, y + h - 1, color);
        }
        for (auto dy = 0; dy < h; dy += 1) {
            graphic_pixel(x, y + dy, color);
            graphic_pixel(x + w - 1, y + dy, color);
        }

        if (fill) {
            auto l = std::clamp(x + 1, 0, 128);
            auto r = std::clamp(x + w - 1, 0, 128);
            auto t = std::clamp(y + 1, 0, 128);
            auto b = std::clamp(y + h - 1, 0, 128);

            for (auto y = t; y < b; y += 1) {
                for (auto x = l; x < r; x += 1) {
                    graphic_pixel(x, y, color);
                }
            }
        }
    }

}
