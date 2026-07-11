#pragma once
#include <stdint.h>

namespace t8::input::mouse
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

    void flush(MouseState &s);

    void move(MouseState &s, int16_t x, int16_t y);

    void button(MouseState &s, uint8_t btn, bool down);

    void wheel(MouseState &s, int16_t z);

    bool clicked(MouseState &s, int x, int y, int w, int h, uint8_t btn = 1);

    bool dragging(MouseState &s, int x, int y, int w, int h, uint8_t btn = 1);

    bool pressed(const MouseState &s, uint8_t btn);

    bool inside(const MouseState &s, int x, int y, int w, int h);

    bool down(const MouseState &s, uint8_t btn);

}