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

    void keybd_flush(KeyboardState &state);

    void keybd_button(KeyboardState &state, uint8_t btn, uint16_t mod, bool repeat, bool down);

    bool keybd_down(const KeyboardState &state, uint8_t btn);

    bool keybd_pressed(const KeyboardState &state, uint8_t btn);

    bool keybd_repeated(const KeyboardState &state, uint8_t btn);

    bool keybd_released(const KeyboardState &state, uint8_t btn);

    bool keybd_triggered(const KeyboardState &state, uint8_t btn);

    bool keybd_ctrl(const KeyboardState &state);

    bool keybd_shift(const KeyboardState &state);

    bool keybd_alt(const KeyboardState &state);

    bool keybd_capslock(const KeyboardState &state);
}