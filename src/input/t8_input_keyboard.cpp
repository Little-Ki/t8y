#include "t8_input_keyboard.h"
#include <memory>

namespace t8::input {

    struct KeyboardState {
        uint8_t button[32]{0};
        uint8_t pressed[32]{0};
        uint8_t released[32]{0};
        uint8_t repeated[32]{0};
        uint8_t mod{0};
    };

    KeyboardState state;

    void keyboard_flush() {
        std::memset(state.pressed, 0, sizeof(state.pressed));
        std::memset(state.released, 0, sizeof(state.released));
        std::memset(state.repeated, 0, sizeof(state.repeated));
    }

    void keyboard_button(uint8_t btn, uint16_t mod, bool repeat, bool down) {
        auto i = btn >> 3;
        auto &button = state.button[i];
        auto &pressed = state.pressed[i];
        auto &released = state.released[i];
        auto &repeated = state.repeated[i];

        if (down) {
            button |= 1 << (btn & 0b111);
            if (!repeat)
                pressed |= 1 << (btn & 0b111);
        } else {
            button &= ~(1 << (btn & 0b111));
            released |= 1 << (btn & 0b111);
        }

        if (repeat) {
            repeated |= 1 << (btn & 0b111);
        }

        state.mod = mod & 0xFF;
    }

    bool keyboard_down(uint8_t btn) {
        return state.button[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keyboard_pressed(uint8_t btn) {
        return state.pressed[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keyboard_repeated(uint8_t btn) {
        return state.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keyboard_released(uint8_t btn) {
        return state.released[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keyboard_triggered(uint8_t btn) {
        return keyboard_pressed(btn) || keyboard_repeated(btn);
    }

    bool keyboard_ctrl() {
        return state.mod & (0x0040 | 0x0080);
    }

    bool keyboard_shift() {
        return state.mod & (0x0001 | 0x0002);
    }

    bool keyboard_alt() {
        return state.mod & (0x0100 | 0x0200);
    }

    bool keyboard_capslock() {
        return state.mod & 0x2000;
    }

}