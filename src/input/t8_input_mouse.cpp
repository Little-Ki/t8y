#include "t8_input_mouse.h"

namespace t8::input {

    struct MouseState {
        uint8_t x{0}, y{0};
        int8_t z{0};
        uint8_t button{0};
        uint8_t pressed{0};
        uint8_t released{0};
        int8_t dx{0}, dy{0};
    };

    struct MouseObserver {
        int index, focus;
    };

    MouseObserver ob[3]{};

    MouseState state;

    void mouse_flush() {
        for (auto i = 1; i <= 3; i++)
            if (!mouse_down(i) && ob[i - 1].focus)
                ob[i - 1].focus = 0;

        state.dx = 0;
        state.dy = 0;
        state.pressed = 0;
        state.released = 0;
        state.z = 0;

        for (auto &o : ob)
            o.index = 0;
    }

    void mouse_move(uint8_t x, uint8_t y) {
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
            state.released |= (1 << (btn - 1));
        }
    }

    void mouse_wheel(int8_t z) {
        state.z = z;
    }

    bool mouse_pressed(uint8_t btn) {
        return state.pressed & (0x1 << (btn - 1));
    }

    bool mouse_released(uint8_t btn) {
        return state.released & (0x1 << (btn - 1));
    }

    bool mouse_clicked(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++ob[btn - 1].index;
        auto px = state.x, py = state.y;

        if (mouse_pressed(btn) && !ob[btn - 1].focus) {
            if (mouse_inside(x, y, w, h))
                ob[btn - 1].focus = id;
        } else if (ob[btn - 1].focus == id && mouse_released(btn)) {
            ob[btn - 1].focus = 0;
            if (mouse_inside(x, y, w, h))
                return true;
        }

        return false;
    }

    bool mouse_dragging(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++ob[btn - 1].index;
        auto px = state.x, py = state.y;

        if (mouse_pressed(btn) && !ob[btn - 1].focus) {
            if (mouse_inside(x, y, w, h))
                ob[btn - 1].focus = id;
        } else if (ob[btn - 1].focus == id && mouse_down(btn) && mouse_inside(x, y, w, h)) {
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

    const int8_t &mouse_dx() {
        return state.dx;
    }

    const int8_t &mouse_dy() {
        return state.dy;
    }

    const uint8_t &mouse_x() {
        return state.x;
    }

    const uint8_t &mouse_y() {
        return state.y;
    }

    const int8_t &mouse_z() {
        return state.z;
    }

    const uint8_t &mouse_button() {
        return state.button;
    }
}