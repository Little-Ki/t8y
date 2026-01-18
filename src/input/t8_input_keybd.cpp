#include "t8_input_keybd.h"
#include <memory>

namespace t8::input {

    struct KeyboardState {
        uint8_t button[32]{0};
        uint8_t pressed[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

    static KeyboardState keybd_state;

    void keybd_flush() {
        std::memset(keybd_state.pressed, 0, sizeof(keybd_state.pressed));
        std::memset(keybd_state.repeated, 0, sizeof(keybd_state.repeated));
    }

    void keybd_button(uint8_t btn, uint16_t mod, bool repeat, bool down) {
        auto i = btn >> 3;
        auto &button = keybd_state.button[i];
        auto &pressed = keybd_state.pressed[i];
        auto &repeated = keybd_state.repeated[i];

        if (down) {
            button |= 1 << (btn & 0b111);
            if (!repeat)
                pressed |= 1 << (btn & 0b111);
        } else {
            button &= ~(1 << (btn & 0b111));
        }

        if (repeat) {
            repeated |= 1 << (btn & 0b111);
        }

        keybd_state.mod = mod & 0xFF;
    }

    bool keybd_down(uint8_t btn) {
        return keybd_state.button[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_pressed(uint8_t btn) {
        return keybd_state.pressed[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_repeated(uint8_t btn) {
        return keybd_state.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_triggered(uint8_t btn) {
        return keybd_pressed(btn) || keybd_repeated(btn);
    }

    bool keybd_ctrl() {
        return keybd_state.mod & (0x0040 | 0x0080);
    }

    bool keybd_shift() {
        return keybd_state.mod & (0x0001 | 0x0002);
    }

    bool keybd_alt() {
        return keybd_state.mod & (0x0100 | 0x0200);
    }

    bool keybd_capslock() {
        return keybd_state.mod & 0x2000;
    }

}