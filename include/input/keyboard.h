#pragma once

#include <cstdint>

namespace t8::input::keyboard
{

    struct KeyboardState
    {
        uint8_t previous[32]{0};
        uint8_t current[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

    void flush(KeyboardState &s) noexcept;

    void button(KeyboardState &s, uint8_t btn, uint16_t mod, bool repeat, bool down);

    bool down(const KeyboardState &s, uint8_t btn);

    bool pressed(const KeyboardState &s, uint8_t btn);

    bool repeated(const KeyboardState &s, uint8_t btn);

    bool released(const KeyboardState &s, uint8_t btn);

    bool triggered(const KeyboardState &s, uint8_t btn);

    bool ctrl(const KeyboardState &s);

    bool shift(const KeyboardState &s);

    bool alt(const KeyboardState &s);

    bool capslock(const KeyboardState &s);
}