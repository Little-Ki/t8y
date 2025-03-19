#include "t8_keybd.h"

namespace t8 {
    
    KeyboardState state;

    void keybd_flush() {
        for (auto i = 0; i < 32; i++) {
            state.pressed[i] = 0;
            state.released[i] = 0;
            state.repeated[i] = 0;
        }
    }

    void keybd_button(uint8_t btn, uint16_t mod, bool repeat, bool down) {
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

    bool keybd_down(uint8_t btn) {
        return state.button[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_pressed(uint8_t btn) {
        return state.pressed[btn >> 3] & (1 << (btn & 0b111));
    }
    
    bool keybd_repeated(uint8_t btn) {
        return state.repeated[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_released(uint8_t btn) {
        return state.released[btn >> 3] & (1 << (btn & 0b111));
    }

    bool keybd_triggered(uint8_t btn) {
        return keybd_pressed(btn) || keybd_repeated(btn);
    }

    bool keybd_ctrl() {
        return state.mod & (0x0040 | 0x0080);
    }

    bool keybd_shift() {
        return state.mod & (0x0001 | 0x0002);
    }

    bool keybd_alt() {
        return state.mod & (0x0100 | 0x0200);
    }

    bool keybd_capslock() {
        return state.mod & 0x2000;
    }

}