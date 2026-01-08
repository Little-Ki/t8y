#pragma once

#include <stdint.h>

namespace t8::input {
    void gamepad_flush();

    void gamepad_join(uint32_t id);

    void gamepad_remove(uint32_t id);

    void gamepad_button(uint32_t id, uint8_t button, bool down);

    bool gamepad_down(uint8_t i, uint8_t button);

    bool gamepad_pressed(uint8_t i, uint8_t button);
}