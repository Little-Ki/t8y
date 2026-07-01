#pragma once
#include <stdint.h>

namespace t8::input
{

    struct MouseState
    {
        struct MouseTracker
        {
            int index, focus;
        };

        int16_t x{0}, y{0};
        int16_t z{0};
        uint8_t current{0};
        uint8_t previous{0};
        int16_t dx{0}, dy{0};
        MouseTracker tracker[3];
    };

    void mouse_flush(MouseState &state);

    void mouse_move(MouseState &state, int16_t x, int16_t y);

    void mouse_button(MouseState &state, uint8_t btn, bool down);

    void mouse_wheel(MouseState &state, int16_t z);

    bool mouse_pressed(const MouseState &state, uint8_t btn);

    bool mouse_clicked(MouseState &state, int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_dragging(MouseState &state, int x, int y, int w, int h, uint8_t btn = 1);

    bool mouse_inside(const MouseState &state, int x, int y, int w, int h);

    bool mouse_down(const MouseState &state, uint8_t btn);

}