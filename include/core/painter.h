#pragma once
#include <stdint.h>

namespace t8::core
{

    struct VirtualMemory;

    void painter_reset(VirtualMemory &mem);

    void painter_clip(VirtualMemory &mem, int x = 0, int y = 0, int w = 128, int h = 128);

    void painter_camera(VirtualMemory &mem, int8_t x = 0, int8_t y = 0);

    void painter_clear(VirtualMemory &mem, uint8_t c);

    void painter_palette_mask(VirtualMemory &mem, uint8_t color, bool t);

    void painter_palette_mask(VirtualMemory &mem, uint16_t t = 1);

    void painter_reset_palette(VirtualMemory &mem);

    void painter_palette(VirtualMemory &mem, uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    void painter_palette(VirtualMemory &mem, uint8_t n, uint8_t map);

    uint8_t painter_palette(VirtualMemory &mem, uint8_t n);

    void painter_pixel(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t painter_pixel(VirtualMemory &mem, int x, int y);

    void painter_sprite(VirtualMemory &mem, int x, int y, uint8_t color);

    uint8_t painter_sprite(VirtualMemory &mem, int x, int y);

    void painter_map(VirtualMemory &mem, int x, int y, uint8_t n);

    uint8_t painter_map(VirtualMemory &mem, int x, int y);

    void painter_flags(VirtualMemory &mem, uint8_t n, uint8_t f);

    uint8_t painter_flags(VirtualMemory &mem, uint8_t n);

    void painter_font(VirtualMemory &mem, int x, int y, bool value, bool custom);

    bool painter_font(VirtualMemory &mem, int x, int y, bool custom);

    void painter_char(VirtualMemory &mem, uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void painter_line(VirtualMemory &mem, int x0, int y0, int x1, int y1, uint8_t color);

    void painter_circle(VirtualMemory &mem, int xc, int yc, int r, uint8_t color, bool fill = false);

    void painter_rect(VirtualMemory &mem, int x, int y, int w, int h, uint8_t color, bool fill = false);

}