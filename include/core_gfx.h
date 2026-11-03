#pragma once
#include <stdint.h>

namespace t8::core
{
    struct VirtualMemory;
}

namespace t8::core::gfx
{
    using namespace t8::core;

    void reset_all(VirtualMemory &mem);

    void set_clip(VirtualMemory &mem, int x = 0, int y = 0, int w = 128, int h = 128);

    void set_offset(VirtualMemory &mem, int8_t x = 0, int8_t y = 0);

    void clear(VirtualMemory &mem, uint8_t c);

    void set_trans(VirtualMemory &mem, uint8_t color, bool t);

    void set_trans(VirtualMemory &mem, uint16_t t = 1);

    void reset_palette(VirtualMemory &mem);

    void set_pcolor(VirtualMemory &mem, uint8_t index, uint32_t c);

    void set_pmap(VirtualMemory &mem, uint8_t n, uint8_t map);

    uint8_t get_pmap(VirtualMemory &mem, uint8_t n);

    void set_pixel(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t get_pixel(VirtualMemory &mem, int x, int y);

    void set_sprite(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t get_sprite(VirtualMemory &mem, int x, int y);

    void set_tilemap(VirtualMemory &mem, int x, int y, uint8_t n);

    uint8_t get_tilemap(VirtualMemory &mem, int x, int y);

    void set_flags(VirtualMemory &mem, uint8_t n, uint8_t f);

    uint8_t get_flags(VirtualMemory &mem, uint8_t n);

    void set_font(VirtualMemory &mem, int x, int y, bool value, bool custom);

    bool get_font(VirtualMemory &mem, int x, int y, bool custom);

    void draw_char(VirtualMemory &mem, uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void draw_line(VirtualMemory &mem, int x0, int y0, int x1, int y1, uint8_t color);

    void draw_circle(VirtualMemory &mem, int xc, int yc, int r, uint8_t color, bool fill = false);

    void draw_rect(VirtualMemory &mem, int x, int y, int w, int h, uint8_t color, bool fill = false);

}