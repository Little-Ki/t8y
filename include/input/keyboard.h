#pragma once

#include <cstdint>

namespace t8::input
{

    struct KeyboardState
    {
        uint8_t previous[32]{0};
        uint8_t current[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

    void k_flush(KeyboardState &s);

    void k_button(KeyboardState &s, uint8_t btn, uint16_t mod, bool repeat, bool down);

    bool k_down(const KeyboardState &s, uint8_t btn);

    bool k_pressed(const KeyboardState &s, uint8_t btn);

    bool k_repeated(const KeyboardState &s, uint8_t btn);

    bool k_released(const KeyboardState &s, uint8_t btn);

    bool k_triggered(const KeyboardState &s, uint8_t btn);

    bool k_ctrl(const KeyboardState &s);

    bool k_shift(const KeyboardState &s);

    bool k_alt(const KeyboardState &s);

    bool k_capslock(const KeyboardState &s);
}