#include "input/mouse.h"

namespace t8::input
{
    void m_flush(MouseState &s)
    {
        for (auto i = 1; i <= 3; i++)
            if (!m_down(s, i) && s.tracker[i - 1].focus)
                s.tracker[i - 1].focus = 0;

        for (auto &x : s.tracker)
            x.index = 0;

        s.dx = 0;
        s.dy = 0;
        s.z = 0;
        s.previous = s.current;
    }

    void m_move(MouseState &s, int16_t x, int16_t y)
    {
        s.dx += x - s.x;
        s.dy += y - s.y;
        s.x = x;
        s.y = y;
    }

    void m_button(MouseState &s, uint8_t btn, bool down)
    {
        if (down)
        {
            s.current |= (1 << (btn - 1));
        }
        else
        {
            s.current &= ~(1 << (btn - 1));
        }
    }

    void m_wheel(MouseState &s, int16_t z)
    {
        s.z = z;
    }

    bool m_pressed(const MouseState &s, uint8_t btn)
    {
        return s.current & (0x1 << (btn - 1));
    }

    bool m_clicked(MouseState &s, int x, int y, int w, int h, uint8_t btn)
    {
        if (btn & 0b11111100)
            return false;

        auto tracker = &s.tracker[btn - 1];
        auto id = ++(tracker->index);
        auto px = s.x, py = s.y;

        if (m_pressed(s, btn) && !tracker->focus)
        {
            if (m_inside(s, x, y, w, h))
                tracker->focus = id;
        }
        else if (tracker->focus == id && !m_down(s, btn))
        {
            tracker->focus = 0;
            if (m_inside(s, x, y, w, h))
                return true;
        }

        return false;
    }

    bool m_dragging(MouseState &s, int x, int y, int w, int h, uint8_t btn)
    {
        if (btn & 0b11111100)
            return false;

        auto tracker = &s.tracker[btn - 1];
        auto id = ++(tracker->index);
        auto px = s.x, py = s.y;

        if (m_pressed(s, btn) && !tracker->focus)
        {
            if (m_inside(s, x, y, w, h))
                tracker->focus = id;
        }
        else if (tracker->focus == id && m_down(s, btn) && m_inside(s, x, y, w, h))
        {
            return true;
        }

        return false;
    }

    bool m_inside(const MouseState &s, int x, int y, int w, int h)
    {
        const auto px = s.x, py = s.y;
        return px >= x && py >= y && px < (x + w) && py < (y + h);
    }

    bool m_down(const MouseState &s, uint8_t btn)
    {
        return s.current & (1 << (btn - 1));
    }

}