#pragma once
#include <stdint.h>

namespace t8::input {
    void mouse_flush(bool clear_pressed = true);

    void mouse_move(int16_t x, int16_t y);

    void mouse_button(uint8_t btn, bool down);

    void mouse_wheel(int16_t z);

    bool mouse_pressed(uint8_t btn);

    bool mouse_clicked(int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_dragging(int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_inside(int x, int y, int w, int h);

    bool mouse_down(uint8_t btn);

    const int16_t &mouse_dx();

    const int16_t &mouse_dy();

    const int16_t &mouse_x();

    const int16_t &mouse_y();

    const int16_t &mouse_z();

    const uint8_t &mouse_button();
}