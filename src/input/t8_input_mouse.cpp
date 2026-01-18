#include "t8_input_mouse.h"

namespace t8::input {

    struct MouseState {
        int16_t x{0}, y{0};
        int16_t z{0};
        uint8_t button{0};
        uint8_t pressed{0};
        int16_t dx{0}, dy{0};
    };

    struct MouseObserver {
        int index, focus;
    };

    static MouseObserver ovservers[3]{};

    static MouseState state;

    void mouse_flush(bool clear_pressed) {
        for (auto i = 1; i <= 3; i++)
            if (!mouse_down(i) && ovservers[i - 1].focus)
                ovservers[i - 1].focus = 0;

        for (auto &ob : ovservers)
            ob.index = 0;

        state.dx = 0;
        state.dy = 0;
        state.z = 0;

        if (clear_pressed) {
            state.pressed = 0;
        }
    }

    void mouse_move(int16_t x, int16_t y) {
        state.dx = x - state.x;
        state.dy = y - state.y;
        state.x = x;
        state.y = y;
    }

    void mouse_button(uint8_t btn, bool down) {
        if (down) {
            state.button |= (1 << (btn - 1));
            state.pressed |= (1 << (btn - 1));
        } else {
            state.button &= ~(1 << (btn - 1));
        }
    }

    void mouse_wheel(int16_t z) {
        state.z = z;
    }

    bool mouse_pressed(uint8_t btn) {
        return state.pressed & (0x1 << (btn - 1));
    }

    bool mouse_clicked(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++ovservers[btn - 1].index;
        auto px = state.x, py = state.y;

        if (mouse_pressed(btn) && !ovservers[btn - 1].focus) {
            if (mouse_inside(x, y, w, h))
                ovservers[btn - 1].focus = id;
        } else if (ovservers[btn - 1].focus == id && !mouse_down(btn)) {
            ovservers[btn - 1].focus = 0;
            if (mouse_inside(x, y, w, h))
                return true;
        }

        return false;
    }

    bool mouse_dragging(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++ovservers[btn - 1].index;
        auto px = state.x, py = state.y;

        if (mouse_pressed(btn) && !ovservers[btn - 1].focus) {
            if (mouse_inside(x, y, w, h))
                ovservers[btn - 1].focus = id;
        } else if (ovservers[btn - 1].focus == id && mouse_down(btn) && mouse_inside(x, y, w, h)) {
            return true;
        }

        return false;
    }

    bool mouse_inside(int x, int y, int w, int h) {
        const auto px = state.x, py = state.y;
        return px >= x && py >= y && px < (x + w) && py < (y + h);
    }

    bool mouse_down(uint8_t btn) {
        return state.button & (1 << (btn - 1));
    }

    const int16_t &mouse_dx() {
        return state.dx;
    }

    const int16_t &mouse_dy() {
        return state.dy;
    }

    const int16_t &mouse_x() {
        return state.x;
    }

    const int16_t &mouse_y() {
        return state.y;
    }

    const int16_t &mouse_z() {
        return state.z;
    }

    const uint8_t &mouse_button() {
        return state.button;
    }
}