#pragma once
#include <stdint.h>

namespace t8 {

    struct MouseState {
        uint8_t x{0}, y{0};
        int8_t z{0};
        uint8_t button{0};
        uint8_t pressed{0};
        uint8_t released{0};
        int8_t dx{0}, dy{0};
    };

    void mouse_flush();

    void mouse_move(uint8_t x, uint8_t y);

    void mouse_button(uint8_t btn, bool down);

    void mouse_wheel(int8_t z);

    bool mouse_pressed(uint8_t btn);

    bool mouse_released(uint8_t btn);

    bool mouse_clicked(int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_dragging(int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_inside(int x, int y, int w, int h);

    bool mouse_down(uint8_t btn);

    const int8_t &mouse_dx();
    
    const int8_t &mouse_dy();

    const uint8_t &mouse_x();

    const uint8_t &mouse_y();

    const int8_t &mouse_z();

    const uint8_t &mouse_button();
    
}