#pragma once

#include <stdint.h>

namespace t8 {

    struct KeyboardState {
        uint8_t button[32]{0};
        uint8_t pressed[32]{0};
        uint8_t released[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

    void keybd_flush();

    void keybd_button(uint8_t btn, uint16_t mod, bool repeat, bool down);

    bool keybd_down(uint8_t btn);

    bool keybd_pressed(uint8_t btn);

    bool keybd_repeated(uint8_t btn);

    bool keybd_released(uint8_t btn);

    bool keybd_triggered(uint8_t btn);

    bool keybd_ctrl();

    bool keybd_shift();

    bool keybd_alt();

    bool keybd_capslock();

}