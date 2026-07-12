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

    void g_flush(GamepadState &s);

    void g_join(GamepadState &s, uint32_t id);

    void g_remove(GamepadState &s, uint32_t id);

    void g_button(GamepadState &s, uint32_t id, uint8_t btn, bool down);

    bool g_down(const GamepadState &s, uint8_t i, uint8_t btn);

    bool g_pressed(const GamepadState &s, uint8_t i, uint8_t btn);
    
    bool g_released(const GamepadState &s, uint8_t i, uint8_t btn);
}