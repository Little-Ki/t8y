#pragma once
#include <stdint.h>

namespace t8::core
{
    struct VirtualMemory;
}

namespace t8::core
{
    using namespace t8::core;

    void gfx_reset_all(VirtualMemory &mem);

    void gfx_set_clip(VirtualMemory &mem, int x = 0, int y = 0, int w = 128, int h = 128);

    void gfx_set_offset(VirtualMemory &mem, int8_t x = 0, int8_t y = 0);

    void gfx_clear(VirtualMemory &mem, uint8_t c);

    void gfx_set_trans(VirtualMemory &mem, uint8_t color, bool t);

    void gfx_set_trans(VirtualMemory &mem, uint16_t t = 1);

    void gfx_reset_palette(VirtualMemory &mem);

    void gfx_set_pcolor(VirtualMemory &mem, uint8_t index, uint32_t c);

    void gfx_set_pmap(VirtualMemory &mem, uint8_t n, uint8_t map);

    uint8_t gfx_get_pmap(VirtualMemory &mem, uint8_t n);

    void gfx_set_pixel(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t gfx_get_pixel(VirtualMemory &mem, int x, int y);

    void gfx_set_sprite(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t gfx_get_sprite(VirtualMemory &mem, int x, int y);

    void gfx_set_tilemap(VirtualMemory &mem, int x, int y, uint8_t n);

    uint8_t gfx_get_tilemap(VirtualMemory &mem, int x, int y);

    void gfx_set_flags(VirtualMemory &mem, uint8_t n, uint8_t f);

    uint8_t gfx_get_flags(VirtualMemory &mem, uint8_t n);

    void gfx_set_font(VirtualMemory &mem, int x, int y, bool value, bool custom);

    bool gfx_get_font(VirtualMemory &mem, int x, int y, bool custom);

    void gfx_draw_char(VirtualMemory &mem, uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void gfx_draw_line(VirtualMemory &mem, int x0, int y0, int x1, int y1, uint8_t color);

    void gfx_draw_circle(VirtualMemory &mem, int xc, int yc, int r, uint8_t color, bool fill = false);

    void gfx_draw_rect(VirtualMemory &mem, int x, int y, int w, int h, uint8_t color, bool fill = false);

}