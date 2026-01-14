#pragma once
#include <stdint.h>

namespace t8::core {

    void painter_reset();

    void painter_clip(int x = 0, int y = 0, int w = 128, int h = 128);

    void painter_camera(int8_t x = 0, int8_t y = 0);

    void painter_clear(uint8_t c);

    void painter_palette_mask(uint8_t color, bool t);

    void painter_palette_mask(uint16_t t = 1);

    void painter_reset_palette();

    void painter_palette(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    void painter_palette(uint8_t n, uint8_t map);

    uint8_t painter_palette(uint8_t n);

    void painter_pixel(int x, int y, uint8_t color);

    uint8_t painter_pixel(int x, int y);

    void painter_sprite(int x, int y, uint8_t color);

    uint8_t painter_sprite(int x, int y);

    void painter_map(int x, int y, uint8_t n);

    uint8_t painter_map(int x, int y);

    void painter_flags(uint8_t n, uint8_t f);

    uint8_t painter_flags(uint8_t n);

    void painter_font(int x, int y, bool value, bool custom);

    bool painter_font(int x, int y, bool custom);

    void painter_char(uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void painter_line(int x0, int y0, int x1, int y1, uint8_t color);

    void painter_circle(int xc, int yc, int r, uint8_t color, bool fill = false);

    void painter_rect(int x, int y, int w, int h, uint8_t color, bool fill = false);

}