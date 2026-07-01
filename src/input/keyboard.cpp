#include "input/keyboard.h"
#include <memory>

namespace t8::input
{
    void keybd_flush(KeyboardState &state)
    {
        std::memcpy(state.previous, state.current, 32);
        std::memset(state.repeated, 0, sizeof(state.repeated));
    }

    void keybd_button(KeyboardState &state, uint8_t btn, uint16_t mod, bool repeat, bool down)
    {
        auto i = btn >> 3;
        auto &current = state.current[i];
        auto &repeated = state.repeated[i];

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

        state.mod = mod & 0xFF;
    }

    bool keybd_down(const KeyboardState &state, uint8_t btn)
    {
        return state.current[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_pressed(const KeyboardState &state, uint8_t btn)
    {
        return (state.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((state.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool keybd_released(const KeyboardState &state, uint8_t btn)
    {
        return (state.current[btn >> 3] & (1 << (btn & 0b111))) &&
               !((state.previous[btn >> 3] & (1 << (btn & 0b111))));
    }

    bool keybd_repeated(const KeyboardState &state, uint8_t btn)
    {
        return state.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_triggered(const KeyboardState &state, uint8_t btn)
    {
        return keybd_pressed(state, btn) || keybd_repeated(state, btn);
    }

    bool keybd_ctrl(const KeyboardState &state)
    {
        return state.mod & (0x0040 | 0x0080);
    }

    bool keybd_shift(const KeyboardState &state)
    {
        return state.mod & (0x0001 | 0x0002);
    }

    bool keybd_alt(const KeyboardState &state)
    {
        return state.mod & (0x0100 | 0x0200);
    }

    bool keybd_capslock(const KeyboardState &state)
    {
        return state.mod & 0x2000;
    }

}