#include "input/keyboard.h"
#include <memory>

namespace t8::input
{
    void k_flush(KeyboardState &s)
    {
        std::memcpy(s.previous, s.current, 32);
        std::memset(s.repeated, 0, sizeof(s.repeated));
    }

    void k_button(KeyboardState &s, uint8_t btn, uint16_t mod, bool repeat, bool down)
    {
        auto i = btn >> 3;
        auto &current = s.current[i];
        auto &repeated = s.repeated[i];

        if (down)
        {
            current |= 1 << (btn & 0b111);
        }
        else
        {
            current &= ~(1 << (btn & 0b111));
        }

        if (repeat)
        {
            repeated |= 1 << (btn & 0b111);
        }

        s.mod = mod & 0xFF;
    }

    bool k_down(const KeyboardState &s, uint8_t btn)
    {
        return s.current[btn >> 3] & (1 << (btn & 0b111));
    }

    bool k_pressed(const KeyboardState &s, uint8_t btn)
    {
        return (s.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((s.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool k_released(const KeyboardState &s, uint8_t btn)
    {
        return (s.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((s.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool k_repeated(const KeyboardState &s, uint8_t btn)
    {
        return s.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool k_triggered(const KeyboardState &s, uint8_t btn)
    {
        return k_pressed(s, btn) || k_repeated(s, btn);
    }

    bool k_ctrl(const KeyboardState &s)
    {
        return s.mod & (0x0040 | 0x0080);
    }

    bool k_shift(const KeyboardState &s)
    {
        return s.mod & (0x0001 | 0x0002);
    }

    bool k_alt(const KeyboardState &s)
    {
        return s.mod & (0x0100 | 0x0200);
    }

    bool k_capslock(const KeyboardState &s)
    {
        return s.mod & 0x2000;
    }

}