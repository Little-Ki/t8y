#pragma once

#include <cstdint>

namespace t8::input {
    
    void keyboard_flush();

    void keyboard_button(uint8_t btn, uint16_t mod, bool repeat, bool down);

    bool keyboard_down(uint8_t btn);

    bool keyboard_pressed(uint8_t btn);

    bool keyboard_repeated(uint8_t btn);

    bool keyboard_released(uint8_t btn);

    bool keyboard_triggered(uint8_t btn);

    bool keyboard_ctrl();

    bool keyboard_shift();

    bool keyboard_alt();

    bool keyboard_capslock();
}