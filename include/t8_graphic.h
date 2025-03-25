#pragma once
#include <stdint.h>

namespace t8 {

    struct GraphicState {
        uint8_t prev_palette[0x8];
    };

    void graphic_backup_palette();

    void graphic_restore_palette();

    void graphic_reset();

    void graphic_clip(int x = 0, int y = 0, int w = 128, int h = 128);

    void graphic_camera(int8_t x = 0, int8_t y = 0);

    void graphic_clear(uint8_t c);

    void graphic_opacity(uint8_t color, bool t);

    void graphic_opacity(uint16_t t = 1);

    void graphic_reset_palette();
    
    void graphic_palette(uint8_t n, uint8_t map);

    uint8_t graphic_palette(uint8_t n);

    void graphic_pixel(int x, int y, uint8_t color);

    uint8_t graphic_pixel(int x, int y);

    void graphic_sprite(int x, int y, uint8_t color);

    uint8_t graphic_sprite(int x, int y);

    void graphic_map(int x, int y, uint8_t n);

    uint8_t graphic_map(int x, int y);

    void graphic_flags(uint8_t n, uint8_t f);

    uint8_t graphic_flags(uint8_t n);

    void graphic_font(int x, int y, bool value, bool custom);

    bool graphic_font(int x, int y, bool custom);

    void graphic_char(uint8_t n, int x, int y, uint8_t color = 0x1, bool custom = false);

    void graphic_line(int x0, int y0, int x1, int y1, uint8_t color);

    void graphic_circle(int xc, int yc, int r, uint8_t color, bool fill = false);

    void graphic_rect(int x, int y, int w, int h, uint8_t color, bool fill = false);

}