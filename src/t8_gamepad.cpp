#include "t8_gamepad.h"

#include <algorithm>

namespace t8 {
    GamepadState state;

    uint32_t mapper[4]{0};

    void gamepad_flush() {
        state.pressed[0].button = 0;
        state.pressed[1].button = 0;
        state.pressed[2].button = 0;
        state.pressed[3].button = 0;
    }

    void gamepad_join(uint32_t id) {
        for (auto &i : mapper) {
            if (i == 0) {
                i = id;
                break;
            }
        }
    }

    void gamepad_remove(uint32_t id) {
        for (auto &i : mapper) {
            if (i == id) {
                i = 0;
                break;
            }
        }

        for (auto i = 0; i < 4; i++) {
            if (mapper[i] != 0)
                continue;

            for (auto j = i + 1; j < 4; j++) {
                if (mapper[j] == 0)
                    continue;
                std::swap(mapper[i], mapper[j]);
                break;
            }
        }
    }

    void gamepad_button(uint32_t id, uint8_t button, bool down) {
        for (auto i = 0; i < 4; i++) {
            if (mapper[i] == id) {
                if (down) {
                    state.p[i].button |= button;
                    state.pressed[i].button |= button;
                } else {
                    state.p[i].button &= ~button;
                }
                break;
            }
        }
    }

    bool gamepad_down(uint8_t i, uint8_t button) {
        if (i > 3) return false;
        return state.p[i].button & button;
    }

    bool gamepad_pressed(uint8_t i, uint8_t button) {
        if (i > 3) return false;
        return state.pressed[i].button & button;
    }
}