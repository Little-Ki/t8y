#pragma once

#include <cstdint>
#include <string>

namespace t8y {

    struct MouseState {
        uint8_t x{0}, y{0};
        int8_t z{0};
        uint8_t button{0};
        uint8_t pressed{0};
        uint8_t released{0};
        int8_t dx{0}, dy{0};
    };

    struct KeyboardState {
        uint8_t button[32]{0};
        uint8_t pressed[32]{0};
        uint8_t released[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

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

    struct AppContext {
        uint8_t screen[0x2000];
        uint8_t sprite[0x2000];
        uint8_t map[0x4000];
        uint8_t palette[0x8];
        uint16_t trans;
        uint8_t flag[0x100];
        uint8_t fonts[2][0x800];

        uint8_t clip[4];
        int8_t offsets[2];

        MouseState mouse;
        KeyboardState keybd;
        GamepadState gamepad;

        std::string script;
    };

};