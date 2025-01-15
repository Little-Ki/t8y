#include "graphic.h"
#include <algorithm>
#include <functional>

#include "buffer.h"
#include "utils.h"

namespace t8y {

    void Graphic::clear(uint8_t c) {
        c = (c & 0xF) | ((c & 0xF) << 4);
        std::fill(ctx->screen, ctx->screen + 0x2000, c);
    }

    void Graphic::setPalt(uint8_t n, uint8_t remap) {
        if (n & 0xF0)
            return;
        _4BitBuffer<16, 1> t(ctx->palette);
        t.set(n, 0, remap);
    }

    uint8_t Graphic::getPalt(uint8_t n) {
        if (n & 0xF0)
            return 0;
        _4BitBuffer<16, 1> t(ctx->palette);
        return t.get(n, 0);
    }

    void Graphic::setTrans(uint8_t color, bool t) {
        if (color & 0xF0)
            return;
        if (t) {
            ctx->trans |= (1 < (color & 0xF));
        } else {
            ctx->trans &= ~static_cast<uint16_t>((1 < (color & 0xF)));
        }
    }

    void Graphic::setTrans(uint16_t t) {
        ctx->trans = t;
    }

    void Graphic::drawLine(int x0, int y0, int x1, int y1, uint8_t color) {
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
                setPixel(x, y, color);
                if ((error += delta) > dx) {
                    y += fy;
                    error -= dx * 2;
                }
            }
        } else {
            for (; y != y1; y += fy) {
                setPixel(x, y, color);
                if ((error += delta) > dy) {
                    x += fx;
                    error -= dy * 2;
                }
            }
        }
    }

    void Graphic::drawCircel(int xc, int yc, int r, uint8_t color, bool fill) {

        const auto put = [&](int xc, int yc, int x, int y) {
            setPixel(xc + x, yc + y, color);
            setPixel(xc - x, yc + y, color);
            setPixel(xc + x, yc - y, color);
            setPixel(xc - x, yc - y, color);
            setPixel(xc + y, yc + x, color);
            setPixel(xc - y, yc + x, color);
            setPixel(xc + y, yc - x, color);
            setPixel(xc - y, yc - x, color);
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

    void Graphic::drawRect(int x, int y, int w, int h, uint8_t color, bool fill) {
        for (auto dx = 0; dx < w; dx += 1) {
            setPixel(x + dx, y, color);
            setPixel(x + dx, y + h - 1, color);
        }
        for (auto dy = 0; dy < h; dy += 1) {
            setPixel(x, y + dy, color);
            setPixel(x + w - 1, y + dy, color);
        }

        if (fill) {
            auto l = std::clamp(x + 1, 0, 128);
            auto r = std::clamp(x + w - 1, 0, 128);
            auto t = std::clamp(y + 1, 0, 128);
            auto b = std::clamp(y + h - 1, 0, 128);

            for (auto y = t; y < b; y += 1) {
                for (auto x = l; x < r; x += 1) {
                    setPixel(x, y, color);
                }
            }
        }
    }

    void Graphic::drawTTri(int x0, int y0, int z0, int u0, int v0, int x1, int y1, int z1, int u1, int v1, int x2, int y2, int z2, int u2, int v2) {
        struct vert {
            float x, y, z, u, v;
        };

        const auto take = [&](int x, int y) -> uint8_t {
            const auto spriteId = getMap((x / 8), (y / 8));
            const auto color = getSprite(
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

        const auto l = std::clamp(static_cast<int>(utils::min(verts[0].x, verts[1].x, verts[2].x)), 0, 128);
        const auto r = std::clamp(static_cast<int>(utils::max(verts[0].x, verts[1].x, verts[2].x)), 0, 128);
        const auto t = std::clamp(static_cast<int>(utils::min(verts[0].y, verts[1].y, verts[2].y)), 0, 128);
        const auto b = std::clamp(static_cast<int>(utils::max(verts[0].y, verts[1].y, verts[2].y)), 0, 128);

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

                setPixel(p.x, 128 - p.y, take(u, v));
            }
        }
    }

    void Graphic::setPixel(int x, int y, uint8_t color) {
        x += ctx->offsets[0];
        y += ctx->offsets[1];
        if (x < ctx->clip[0] ||
            y < ctx->clip[1] ||
            x >= ctx->clip[0] + ctx->clip[2] ||
            y >= ctx->clip[1] + ctx->clip[3])
            return;

        _4BitBuffer<128, 128> t(ctx->screen);
        t.set(x, y, getPalt(color));
    }

    uint8_t Graphic::getPixel(int x, int y) {
        _4BitBuffer<128, 128> t(ctx->screen);
        return t.get(x, y);
    }

    void Graphic::setSprite(int x, int y, uint8_t color) {
        _4BitBuffer<128, 128> t(ctx->sprite);
        t.set(x, y, color);
    }

    uint8_t Graphic::getSprite(int x, int y) {
        _4BitBuffer<128, 128> t(ctx->sprite);
        return t.get(x, y);
    }

    void Graphic::setMap(int x, int y, uint8_t n) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return;
        auto i = (y * 128 + x);
        ctx->map[i] = n;
    }

    uint8_t Graphic::getMap(int x, int y) {
        if (x < 0 || x >= 128 || y < 0 || y >= 128)
            return 0;
        auto i = (y * 128 + x);
        return ctx->map[i];
    }

    void Graphic::setFlag(uint8_t n, uint8_t flag) {
        ctx->flag[n] = flag;
    }

    uint8_t Graphic::getFlag(uint8_t n) {
        return ctx->flag[n];
    }

    void Graphic::setFont(int x, int y, bool value, bool plane) {
        _1BitBuffer<128, 128> t(ctx->fonts[plane]);
        return t.set(x, y, value);
    }

    bool Graphic::getFont(int x, int y, bool plane) {
        _1BitBuffer<128, 128> t(ctx->fonts[plane]);
        return t.get(x, y);
    }

    void Graphic::drawChar(uint8_t n, int x, int y, uint8_t color, bool plane) {
        auto baseX = (n & 0xF) << 3;
        auto baseY = (n >> 4) << 3;

        for (auto dy = 0; dy < 8; dy++) {
            for (auto dx = 0; dx < 8; dx++) {
                bool c = getFont(baseX + dx, baseY + dy, plane);
                if (c)
                    setPixel(x + dx, y + dy, color);
            }
        }
    }

    Graphic::Graphic(AppContext* ctx) : ctx(ctx) {
    }

    void Graphic::reset() {
        for (auto i = 0; i < 16; i++) {
            setPalt(i, i);
        }
        clip();
    }

    void Graphic::clip(int x, int y, int w, int h) {
        auto l = std::clamp(x, 0, 128);
        auto r = std::clamp(x + w, 0, 128);
        auto t = std::clamp(y, 0, 128);
        auto b = std::clamp(y + h, 0, 128);
        if (l > r || t > b)
            return;

        ctx->clip[0] = static_cast<uint8_t>(l);
        ctx->clip[1] = static_cast<uint8_t>(t);
        ctx->clip[2] = static_cast<uint8_t>(r - l);
        ctx->clip[3] = static_cast<uint8_t>(b - t);
    }

    void Graphic::camera(int8_t x, int8_t y) {
        ctx->offsets[0] = x;
        ctx->offsets[1] = y;
    }

}