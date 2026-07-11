#include "input/keyboard.h"
#include <memory>

namespace t8::input::keyboard
{
    void flush(KeyboardState &s)
    {
        std::memcpy(s.previous, s.current, 32);
        std::memset(s.repeated, 0, sizeof(s.repeated));
    }

    void button(KeyboardState &s, uint8_t btn, uint16_t mod, bool repeat, bool down)
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

    bool down(const KeyboardState &s, uint8_t btn)
    {
        return s.current[btn >> 3] & (1 << (btn & 0b111));
    }

    bool pressed(const KeyboardState &s, uint8_t btn)
    {
        return (s.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((s.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool released(const KeyboardState &s, uint8_t btn)
    {
        return (s.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((s.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool repeated(const KeyboardState &s, uint8_t btn)
    {
        return s.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool triggered(const KeyboardState &s, uint8_t btn)
    {
        return pressed(s, btn) || repeated(s, btn);
    }

    bool ctrl(const KeyboardState &s)
    {
        return s.mod & (0x0040 | 0x0080);
    }

    bool shift(const KeyboardState &s)
    {
        return s.mod & (0x0001 | 0x0002);
    }

    bool alt(const KeyboardState &s)
    {
        return s.mod & (0x0100 | 0x0200);
    }

    bool capslock(const KeyboardState &s)
    {
        return s.mod & 0x2000;
    }

}