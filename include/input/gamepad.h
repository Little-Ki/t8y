#pragma once

#include <stdint.h>

namespace t8::input
{
    struct GamepadState
    {
        union
        {
            struct
            {
                bool up : 1;
                bool down : 1;
                bool left : 1;
                bool right : 1;
                bool a : 1;
                bool b : 1;
                bool x : 1;
                bool y : 1;
            };
            uint8_t btn{0};
        } current[4], previous[4];

        uint32_t mapper[4]{0};
    };

    void gamepad_flush(GamepadState &state);

    void gamepad_join(GamepadState &state, uint32_t id);

    void gamepad_remove(GamepadState &state, uint32_t id);

    void gamepad_button(GamepadState &state, uint32_t id, uint8_t btn, bool down);

    bool gamepad_down(const GamepadState &state, uint8_t i, uint8_t btn);

    bool gamepad_pressed(const GamepadState &state, uint8_t i, uint8_t btn);
    
    bool gamepad_released(const GamepadState &state, uint8_t i, uint8_t btn);
}