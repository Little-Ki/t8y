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

    void m_flush(MouseState &s);

    void m_move(MouseState &s, int16_t x, int16_t y);

    void m_button(MouseState &s, uint8_t btn, bool down);

    void m_wheel(MouseState &s, int16_t z);

    bool m_clicked(MouseState &s, int x, int y, int w, int h, uint8_t btn = 1);

    bool m_dragging(MouseState &s, int x, int y, int w, int h, uint8_t btn = 1);

    bool m_pressed(const MouseState &s, uint8_t btn);

    bool m_inside(const MouseState &s, int x, int y, int w, int h);

    bool m_down(const MouseState &s, uint8_t btn);

}