#include "input.h"
#include "utils.h"
#include <bitset>

#include <iostream>

#include "app.h"

namespace t8y {

    void MouseInput::flush() {
        auto &mouse = ctx->mouse;
        for (auto i = 1; i <= 3; i++)
            if (!down(i) && m_ob[i - 1].focus)
                m_ob[i - 1].focus = 0;
        mouse.dx = 0;
        mouse.dy = 0;
        m_ob[0].index = 0;
        m_ob[1].index = 0;
        m_ob[2].index = 0;
        mouse.pressed = 0;
        mouse.released = 0;
        mouse.z = 0;
    }

    bool MouseInput::pressed(uint8_t btn) {
        return ctx->mouse.pressed & (0x1 << (btn - 1));
    }

    bool MouseInput::released(uint8_t btn) {
        return ctx->mouse.released & (0x1 << (btn - 1));
    }

    bool MouseInput::clicked(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++m_ob[btn - 1].index;
        auto px = this->x(), py = this->y();

        if (pressed(btn) && !m_ob[btn - 1].focus) {
            if (inside(x, y, w, h))
                m_ob[btn - 1].focus = id;
        } else if (m_ob[btn - 1].focus == id && released(btn)) {
            m_ob[btn - 1].focus = 0;
            if (inside(x, y, w, h))
                return true;
        }

        return false;
    }

    bool MouseInput::drag(int x, int y, int w, int h, uint8_t btn) {
        if (btn & 0b11111100)
            return false;

        auto id = ++m_ob[btn - 1].index;
        auto px = this->x(), py = this->y();

        if (pressed(btn) && !m_ob[btn - 1].focus) {
            if (inside(x, y, w, h))
                m_ob[btn - 1].focus = id;
        } else if (m_ob[btn - 1].focus == id && down(btn) && inside(x, y, w, h)) {
            return true;
        }

        return false;
    }

    bool MouseInput::inside(int x, int y, int w, int h) {
        const auto px = this->x(), py = this->y();
        return px >= x && py >= y && px < (x + w) && py < (y + h);
    }

    void MouseInput::move(uint8_t x, uint8_t y) {
        ctx->mouse.dx = x - ctx->mouse.x;
        ctx->mouse.dy = y - ctx->mouse.y;
        ctx->mouse.x = x;
        ctx->mouse.y = y;
    }

    void MouseInput::button(uint8_t b, bool down) {
        if (down) {
            ctx->mouse.button |= (1 << (b - 1));
            ctx->mouse.pressed |= (1 << (b - 1));
        } else {
            ctx->mouse.button &= ~(1 << (b - 1));
            ctx->mouse.released |= (1 << (b - 1));
        }
    }

    bool MouseInput::down(uint8_t b) {
        return ctx->mouse.button & (1 << (b - 1));
    }

    void MouseInput::wheel(int8_t z) {
        ctx->mouse.z = z;
    }

    int8_t MouseInput::dx() const {
        return ctx->mouse.dx;
    }

    int8_t MouseInput::dy() const {
        return ctx->mouse.dy;
    }

    uint8_t MouseInput::x() const {
        return ctx->mouse.x;
    }

    uint8_t MouseInput::y() const {
        return ctx->mouse.y;
    }

    int8_t MouseInput::z() const {
        return ctx->mouse.z;
    }

    uint8_t MouseInput::button() const {
        return ctx->mouse.button;
    }

    void KayboardInput::flush() {
        for (auto i = 0; i < 32; i++) {
            ctx->keybd.pressed[i] = 0;
            ctx->keybd.released[i] = 0;
            ctx->keybd.repeated[i] = 0;
        }
    }

    void KayboardInput::button(uint8_t btn, uint16_t mod, bool repeat, bool down) {
        auto &keybd = ctx->keybd;
        auto i = btn >> 3;
        auto &button = keybd.button[i];
        auto &pressed = keybd.pressed[i];
        auto &released = keybd.released[i];
        auto &_repeat = keybd.repeated[i];

        if (down) {
            button |= 1 << (btn & 0b111);
            if (!repeat)
                pressed |= 1 << (btn & 0b111);
        } else {
            button &= ~(1 << (btn & 0b111));
            released |= 1 << (btn & 0b111);
        }

        if (repeat) {
            _repeat |= 1 << (btn & 0b111);
        }

        keybd.mod = mod & 0xFF;
    }

    bool KayboardInput::down(uint8_t btn) {
        auto i = btn >> 3;
        auto &map = ctx->keybd.button[i];
        return map & (1 << (btn & 0b111));
    }

    bool KayboardInput::pressed(uint8_t btn) {
        auto i = btn >> 3;
        auto &map = ctx->keybd.pressed[i];

        return map & (1 << (btn & 0b111));
    }

    bool KayboardInput::released(uint8_t btn) {
        auto i = btn >> 3;
        auto &map = ctx->keybd.released[i];
        return map & (1 << (btn & 0b111));
    }

    bool KayboardInput::triggered(uint8_t btn) {
        return pressed(btn) || repeated(btn);
    }

    bool KayboardInput::repeated(uint8_t btn) {
        auto i = btn >> 3;
        auto &map = ctx->keybd.repeated[i];
        return map & (1 << (btn & 0b111));
    }

    bool KayboardInput::ctrl() {
        const auto &mod = ctx->keybd.mod;
        return mod & (0x0040 | 0x0080);
    }

    bool KayboardInput::shift() {
        const auto &mod = ctx->keybd.mod;
        return mod & (0x0001 | 0x0002);
    }

    bool KayboardInput::alt() {
        const auto &mod = ctx->keybd.mod;
        return mod & (0x0100 | 0x0200);
    }

    bool KayboardInput::capslock() {
        const auto &mod = ctx->keybd.mod;
        return mod & 0x2000;
    }

    void GamepadInput::flush() {
        ctx->gamepad.pressed[0].button = 0;
        ctx->gamepad.pressed[1].button = 0;
        ctx->gamepad.pressed[2].button = 0;
        ctx->gamepad.pressed[3].button = 0;
    }

    void GamepadInput::add(uint32_t id) {
        for (auto i = 0; i < 4; i++) {
            if (idMapper[i] == 0) {
                idMapper[i] = id;
                break;
            }
        }
    }

    void GamepadInput::remove(uint32_t id) {
        for (auto i = 0; i < 4; i++) {
            if (idMapper[i] == id) {
                idMapper[i] = 0;
                break;
            }
        }

        // shift

        for (auto i = 0; i < 4; i++) {
            if (idMapper[i] != 0)
                continue;

            for (auto j = i + 1; j < 4; j++) {
                if (idMapper[j] == 0)
                    continue;
                std::swap(idMapper[i], idMapper[j]);
                break;
            }
        }
    }

    void GamepadInput::button(uint32_t id, uint8_t button, bool down) {
        for (auto i = 0; i < 4; i++) {
            if (idMapper[i] == id) {
                if (down) {
                    ctx->gamepad.p[i].button |= button;
                    ctx->gamepad.pressed[i].button |= button;

                } else {
                    ctx->gamepad.p[i].button &= ~button;
                }
                break;
            }
        }
    }

}