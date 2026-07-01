#include "input/mouse.h"

namespace t8::input
{

    void mouse_flush(MouseState &state)
    {
        for (auto i = 1; i <= 3; i++)
            if (!mouse_down(state, i) && state.tracker[i - 1].focus)
                state.tracker[i - 1].focus = 0;

        for (auto &x : state.tracker)
            x.index = 0;

        state.dx = 0;
        state.dy = 0;
        state.z = 0;
        state.previous = state.current;
    }

    void mouse_move(MouseState &state, int16_t x, int16_t y)
    {
        state.dx += x - state.x;
        state.dy += y - state.y;
        state.x = x;
        state.y = y;
    }

    void mouse_button(MouseState &state, uint8_t btn, bool down)
    {
        if (down)
        {
            state.current |= (1 << (btn - 1));
        }
        else
        {
            state.current &= ~(1 << (btn - 1));
        }
    }

    void mouse_wheel(MouseState &state, int16_t z)
    {
        state.z = z;
    }

    bool mouse_pressed(const MouseState &state, uint8_t btn)
    {
        return state.current & (0x1 << (btn - 1));
    }

    bool mouse_clicked(MouseState &state, int x, int y, int w, int h, uint8_t btn)
    {
        if (btn & 0b11111100)
            return false;

        auto tracker = &state.tracker[btn - 1];
        auto id = ++(tracker->index);
        auto px = state.x, py = state.y;

        if (mouse_pressed(state, btn) && !tracker->focus)
        {
            if (mouse_inside(state, x, y, w, h))
                tracker->focus = id;
        }
        else if (tracker->focus == id && !mouse_down(state, btn))
        {
            tracker->focus = 0;
            if (mouse_inside(state, x, y, w, h))
                return true;
        }

        return false;
    }

    bool mouse_dragging(MouseState &state, int x, int y, int w, int h, uint8_t btn)
    {
        if (btn & 0b11111100)
            return false;

        auto tracker = &state.tracker[btn - 1];
        auto id = ++(tracker->index);
        auto px = state.x, py = state.y;

        if (mouse_pressed(state, btn) && !tracker->focus)
        {
            if (mouse_inside(state, x, y, w, h))
                tracker->focus = id;
        }
        else if (tracker->focus == id && mouse_down(state, btn) && mouse_inside(state, x, y, w, h))
        {
            return true;
        }

        return false;
    }

    bool mouse_inside(const MouseState &state, int x, int y, int w, int h)
    {
        const auto px = state.x, py = state.y;
        return px >= x && py >= y && px < (x + w) && py < (y + h);
    }

    bool mouse_down(const MouseState &state, uint8_t btn)
    {
        return state.current & (1 << (btn - 1));
    }

}