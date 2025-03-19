#pragma once

#include <stdint.h>

namespace t8 {

    struct GamepadState {
        union {
            struct {
                bool up : 1;
                bool down : 1;
                bool left : 1;
                bool right : 1;
                bool a : 1;
                bool b : 1;
                bool x : 1;
                bool y : 1;
            };
            uint8_t button{0};
        } p[4], pressed[4];
    };

    void gamepad_flush();

    void gamepad_join(uint32_t id);

    void gamepad_remove(uint32_t id);

    void gamepad_button(uint32_t id, uint8_t button, bool down);

    bool gamepad_down(uint8_t i, uint8_t button);
    
    bool gamepad_pressed(uint8_t i, uint8_t button);

}