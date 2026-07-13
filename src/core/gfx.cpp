#include "core/gfx.h"
#include "core/memory.h"
#include "utils/algo.h"

#include <algorithm>
#include <iostream>

namespace t8::core {
    union bitfield_4 {
        uint8_t value;
        struct
        {
            uint8_t hi : 4;
            uint8_t lo : 4;
        };
    };

    void gfx_reset(VirtualMemory *m) {
        gfx_reset_palette(m);
        gfx_clip(m);
        gfx_camera(m);
    }

    void gfx_clip(VirtualMemory *m, int x, int y, int w, int h) {

        auto l = std::clamp(x, 0, 128);
        auto r = std::clamp(x + w, 0, 128);
        auto t = std::clamp(y, 0, 128);
        auto b = std::clamp(y + h, 0, 128);
        if (l > r || t > b)
            return;

        m->view_clip[0] = static_cast<uint8_t>(l);
        m->view_clip[1] = static_cast<uint8_t>(t);
        m->view_clip[2] = static_cast<uint8_t>(r - l);
        m->view_clip[3] = static_cast<uint8_t>(b - t);
    }

    void gfx_camera(VirtualMemory *m, int8_t x, int8_t y) {
        m->draw_offset[0] = x;
        m->draw_offset[1] = y;
    }

    void gfx_clear(VirtualMemory *m, uint8_t c) {
        c = (c & 0xF) | ((c & 0xF) << 4);
        std::fill(m->screen, m->screen + sizeof(m->screen), c);
    }

    void gfx_palt(VirtualMemory *m, uint8_t color, bool t) {
        if (t) {
            m->palette_mask |= (1 << color);
        } else {
            m->palette_mask &= ~static_cast<uint16_t>(1 < color);
        }
    }

    void gfx_palt(VirtualMemory *m, uint16_t t) {
        m->palette_mask = t;
    }

    void gfx_reset_palette(VirtualMemory *m) {
        for (auto i = 0; i < 16; i++) {
            gfx_pal(m, i, i);
        }
    }

    void gfx_palc(VirtualMemory *m, uint8_t index, uint32_t c) {
        m->palette[index & 0xF] = c;
    }

    void gfx_pal(VirtualMemory *m, uint8_t n, uint8_t map) {
        if (n & 0xF0)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(m->palette_mapping);
        auto field = &buffer[n >> 1];
        (n & 0x1) ? (field->lo = map) : (field->hi = map);
    }

    uint8_t gfx_pal(VirtualMemory *m, uint8_t n) {
        if (n & 0xF0)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(m->palette_mapping);
        auto field = &buffer[n >> 1];
        return (n & 0x1) ? (field->lo) : (field->hi);
    }

    void gfx_pset(VirtualMemory *m, int x, int y, uint8_t color) {
        x += m->draw_offset[0];
        y += m->draw_offset[1];
        if (x < m->view_clip[0] ||
            y < m->view_clip[1] ||
            x >= m->view_clip[0] + m->view_clip[2] ||
            y >= m->view_clip[1] + m->view_clip[3])
            return;

        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;

        auto buffer = reinterpret_cast<bitfield_4 *>(m->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        if (!(m->palette_mask & (1 << color))) {
            (t & 1) ? (field->lo = color) : (field->hi = color);
        }
    }

    uint8_t gfx_pget(VirtualMemory *m, int x, int y) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(m->screen);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void gfx_sset(VirtualMemory *m, int x, int y, uint8_t color) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = reinterpret_cast<bitfield_4 *>(m->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        (t & 1) ? (field->lo = color) : (field->hi = color);
    }

    uint8_t gfx_sget(VirtualMemory *m, int x, int y) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return 0;
        auto buffer = reinterpret_cast<bitfield_4 *>(m->sprite);
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 1];
        return (t & 1) ? (field->lo) : (field->hi);
    }

    void gfx_mset(VirtualMemory *m, int x, int y, uint8_t n) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return;
        auto i = (y * 128 + x);
        m->map[i] = n;
    }

    uint8_t gfx_mget(VirtualMemory *m, int x, int y) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return 0;
        auto i = (y * 128 + x);
        return m->map[i];
    }

    void gfx_fset(VirtualMemory *m, uint8_t n, uint8_t f) {
        m->flags[n] = f;
    }

    uint8_t gfx_fget(VirtualMemory *m, uint8_t n) {
        return m->flags[n];
    }

    void gfx_ftset(VirtualMemory *m, int x, int y, bool value, bool custom) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return;
        auto buffer = custom ? m->custom_font : m->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        value ? (*field |= (1 << (t & 0b111))) : (*field &= ~(1 << (t & 0b111)));
    }

    bool gfx_ftget(VirtualMemory *m, int x, int y, bool custom) {
        if (x < 0 || x > 128 || y < 0 || y > 128)
            return false;
        auto buffer = custom ? m->custom_font : m->default_font;
        auto t = (y * 128 + x);
        auto field = &buffer[t >> 3];
        return *field & (1 << (t & 0b111));
    }

    void gfx_char(VirtualMemory *m, uint8_t n, int x, int y, uint8_t color, bool custom) {
        auto sprite_x = (n & 0xF) << 3;
        auto sprite_y = (n >> 4) << 3;

        for (auto dy = 0; dy < 8; dy++) {
            for (auto dx = 0; dx < 8; dx++) {
                bool c = gfx_ftget(m, sprite_x + dx, sprite_y + dy, custom);
                if (c)
                    gfx_pset(m, x + dx, y + dy, color);
            }
        }
    }

    void gfx_line(VirtualMemory *m, int x0, int y0, int x1, int y1, uint8_t color) {
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
                gfx_pset(m, x, y, color);
                if ((error += delta) > dx) {
                    y += fy;
                    error -= dx * 2;
                }
            }
        } else {
            for (; y != y1; y += fy) {
                gfx_pset(m, x, y, color);
                if ((error += delta) > dy) {
                    x += fx;
                    error -= dy * 2;
                }
            }
        }
    }

    void gfx_circ(VirtualMemory *m, int xc, int yc, int r, uint8_t color, bool fill) {
        const auto put = [&](int xc, int yc, int x, int y) {
            gfx_pset(m, xc + x, yc + y, color);
            gfx_pset(m, xc - x, yc + y, color);
            gfx_pset(m, xc + x, yc - y, color);
            gfx_pset(m, xc - x, yc - y, color);
            gfx_pset(m, xc + y, yc + x, color);
            gfx_pset(m, xc - y, yc + x, color);
            gfx_pset(m, xc + y, yc - x, color);
            gfx_pset(m, xc - y, yc - x, color);
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

    void gfx_rect(VirtualMemory *m, int x, int y, int w, int h, uint8_t color, bool fill) {
        for (auto dx = 0; dx < w; dx += 1) {
            gfx_pset(m, x + dx, y, color);
            gfx_pset(m, x + dx, y + h - 1, color);
        }
        for (auto dy = 0; dy < h; dy += 1) {
            gfx_pset(m, x, y + dy, color);
            gfx_pset(m, x + w - 1, y + dy, color);
        }

        if (fill) {
            auto l = std::clamp(x + 1, 0, 128);
            auto r = std::clamp(x + w - 1, 0, 128);
            auto t = std::clamp(y + 1, 0, 128);
            auto b = std::clamp(y + h - 1, 0, 128);

            for (auto y = t; y < b; y += 1) {
                for (auto x = l; x < r; x += 1) {
                    gfx_pset(m, x, y, color);
                }
            }
        }
    }

}
