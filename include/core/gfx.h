#pragma once
#include <stdint.h>

namespace t8::core {
    struct VirtualMemory;
}

namespace t8::core {
    void gfx_reset(VirtualMemory *m);

    void gfx_clip(VirtualMemory *m, int x = 0, int y = 0, int w = 128, int h = 128);

    void gfx_camera(VirtualMemory *m, int8_t x = 0, int8_t y = 0);

    void gfx_clear(VirtualMemory *m, uint8_t c);

    void gfx_palt(VirtualMemory *m, uint8_t c, bool t);

    void gfx_palt(VirtualMemory *m, uint16_t t = 1);

    void gfx_reset_palette(VirtualMemory *m);

    void gfx_palc(VirtualMemory *m, uint8_t index, uint32_t c);

    void gfx_pal(VirtualMemory *m, uint8_t n, uint8_t map);

    uint8_t gfx_pal(VirtualMemory *m, uint8_t n);

    void gfx_pset(VirtualMemory *m, int x, int y, uint8_t c);

    uint8_t gfx_pget(VirtualMemory *m, int x, int y);

    void gfx_sset(VirtualMemory *m, int x, int y, uint8_t c);

    uint8_t gfx_sget(VirtualMemory *m, int x, int y);

    void gfx_mset(VirtualMemory *m, int x, int y, uint8_t n);

    uint8_t gfx_mget(VirtualMemory *m, int x, int y);

    void gfx_fset(VirtualMemory *m, uint8_t n, uint8_t f);

    uint8_t gfx_fget(VirtualMemory *m, uint8_t n);

    void gfx_ftset(VirtualMemory *m, int x, int y, bool value, bool custom);

    bool gfx_ftget(VirtualMemory *m, int x, int y, bool custom);

    void gfx_char(VirtualMemory *m, uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void gfx_line(VirtualMemory *m, int x0, int y0, int x1, int y1, uint8_t color);

    void gfx_circ(VirtualMemory *m, int xc, int yc, int r, uint8_t color, bool fill = false);

    void gfx_rect(VirtualMemory *m, int x, int y, int w, int h, uint8_t color, bool fill = false);

}