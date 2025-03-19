#include "t8_graphic.h"
#include "t8_buffer.h"
#include "t8_memory.h"
#include "t8_utils.h"

#include <algorithm>

namespace t8 {

    void graphic_reset() {
        for (auto i = 0; i < 16; i++) {
            graphic_palette(i, i);
        }
        graphic_clip();
        graphic_camera();
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
        if (color & 0xF0)
            return;
        if (t) {
            mem()->opacity |= (1 < (color & 0xF));
        } else {
            mem()->opacity &= ~static_cast<uint16_t>((1 < (color & 0xF)));
        }
    }

    void graphic_opacity(uint16_t t) {
        mem()->opacity = t;
    }

    void graphic_palette(uint8_t n, uint8_t map) {
        if (n & 0xF0)
            return;
        _4BitBuffer<16, 1> t(mem()->palette);
        t.set(n, 0, map);
    }

    uint8_t graphic_palette(uint8_t n) {
        if (n & 0xF0)
            return 0;
        _4BitBuffer<16, 1> t(mem()->palette);
        return t.get(n, 0);
    }

    void graphic_pixel(int x, int y, uint8_t color) {
        x += mem()->draw_offset[0];
        y += mem()->draw_offset[1];
        if (x < mem()->view_clip[0] ||
            y < mem()->view_clip[1] ||
            x >= mem()->view_clip[0] + mem()->view_clip[2] ||
            y >= mem()->view_clip[1] + mem()->view_clip[3])
            return;

        _4BitBuffer<128, 128> t(mem()->screen);
        t.set(x, y, graphic_palette(color));
    }

    uint8_t graphic_pixel(int x, int y) {
        _4BitBuffer<128, 128> t(mem()->screen);
        return t.get(x, y);
    }

    void graphic_sprite(int x, int y, uint8_t color) {
        _4BitBuffer<128, 128> t(mem()->sprite);
        t.set(x, y, color);
    }

    uint8_t graphic_sprite(int x, int y) {
        _4BitBuffer<128, 128> t(mem()->sprite);
        return t.get(x, y);
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
        _1BitBuffer<128, 128> t(custom ? mem()->custom_font : mem()->default_font);
        return t.set(x, y, value);
    }

    bool graphic_font(int x, int y, bool custom) {
        _1BitBuffer<128, 128> t(custom ? mem()->custom_font : mem()->default_font);
        return t.get(x, y);
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

    void graphic_fragment(int x0, int y0, int z0, int u0, int v0, int x1, int y1, int z1, int u1, int v1, int x2, int y2, int z2, int u2, int v2) {
        struct vert {
            float x, y, z, u, v;
        };

        const auto take = [&](int x, int y) -> uint8_t {
            const auto spriteId = graphic_map((x / 8), (y / 8));
            const auto color = graphic_sprite(
                (spriteId & 0xF) + x % 8,
                (spriteId >> 4) + y % 8);
            return spriteId;
        };

        const auto edge = [](const vert &v0, const vert &v1, const vert p) -> float {
            return (v1.x - v0.x) * (p.y - v0.y) - (v1.y - v0.y) * (p.x - v0.x);
        };

        vert verts[3] = {
            {x0, y0, z0, u0, v0}, {x1, y1, z1, u1, v1}, {x2, y2, z2, u2, v2}};

        for (auto &v : verts) {
            if (v.z == 0)
                v.z = 0.00001f;
            v.z = 1.f / v.z;

            v.x *= v.z;
            v.y *= v.z;
            v.x = (v.x * 0.5 + 0.5) * 128;
            v.y = (v.y * 0.5 + 0.5) * 128;

            v.u *= v.z;
            v.v *= v.z;
        }

        const auto l = std::clamp(static_cast<int>(min(verts[0].x, verts[1].x, verts[2].x)), 0, 128);
        const auto r = std::clamp(static_cast<int>(max(verts[0].x, verts[1].x, verts[2].x)), 0, 128);
        const auto t = std::clamp(static_cast<int>(min(verts[0].y, verts[1].y, verts[2].y)), 0, 128);
        const auto b = std::clamp(static_cast<int>(max(verts[0].y, verts[1].y, verts[2].y)), 0, 128);

        const auto demon = 1.f / edge(verts[0], verts[1], verts[2]);

        for (auto y = t; y < b; y++) {
            for (auto x = l; x < r; x++) {
                vert p = {x + 0.5f, y + 0.5f};

                const float b[3] = {
                    demon * edge(verts[1], verts[2], p),
                    demon * edge(verts[2], verts[0], p),
                    demon * edge(verts[0], verts[1], p)};

                if (b[0] < 0 || b[1] < 0 || b[2] < 0)
                    continue;

                const auto z = 1 / (b[0] * verts[0].z + b[1] * verts[1].z + b[2] * verts[2].z);

                if (z < 1)
                    continue;

                const auto u = z * (b[0] * verts[0].u + b[1] * verts[1].u + b[2] * verts[2].u);
                const auto v = z * (b[0] * verts[0].v + b[1] * verts[1].v + b[2] * verts[2].v);

                graphic_pixel(p.x, 128 - p.y, take(u, v));
            }
        }
    }

}
