#include "t8_vm.h"
#include <algorithm>
#include <chrono>

#include "t8_gamepad.h"
#include "t8_graphic.h"
#include "t8_keybd.h"
#include "t8_memory.h"
#include "t8_mouse.h"
#include "t8_signal.h"

namespace t8 {

    VMState state;

    int vm_call_pmem(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(
                mem()->cache[call.get<uint8_t>(1)]);
        }
        if (call.is<uint8_t, uint32_t>()) {
            mem()->cache[call.get<uint8_t>(1)] = call.get<uint32_t>(2);
        }
        return 0;
    }

    int vm_call_clip(const VMCall &call) {
        if (call.is<int, int, int, int>()) {
            graphic_clip(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4));
        }
        if (call.argc() == 0) {
            graphic_clip();
        }
        return 0;
    }

    int vm_call_pal(const VMCall &call) {
        if (call.is<int, int>()) {
            graphic_palette(call.get<int>(1), call.get<int>(2));
        }
        if (call.is<int>()) {
            auto p = graphic_palette(call.get<int>(1));
            return call.ret(p);
        }
        return 0;
    }

    int vm_call_palt(const VMCall &call) {
        if (call.is<int, bool>()) {
            graphic_opacity(
                call.get<int>(1),
                call.get<bool>(2));
        }
        if (call.is<uint16_t>()) {
            graphic_opacity(
                call.get<uint16_t>(1));
        }
        return 0;
    }

    int vm_call_cls(const VMCall &call) {
        if (call.argc() == 0) {
            graphic_clear(0);
        }
        if (call.is<uint8_t>()) {
            graphic_clear(call.get<uint8_t>(1));
        }
        return 0;
    }

    int vm_call_camera(const VMCall &call) {
        if (call.is<int8_t, int8_t>()) {
            graphic_camera(
                call.get<int8_t>(1),
                call.get<int8_t>(2));
        }
        return 0;
    }

    int vm_call_pget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                graphic_pixel(
                    call.get<int>(1),
                    call.get<int>(2)));
        }

        return 0;
    }

    int vm_call_pset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            graphic_pixel(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int vm_call_fget(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(
                graphic_flags(call.get<uint8_t>(1)));
        }
        return 0;
    }

    int vm_call_fset(const VMCall &call) {
        if (call.is<uint8_t, uint8_t>()) {
            graphic_flags(
                call.get<uint8_t>(1),
                call.get<uint8_t>(2));
        }
        return 0;
    }

    int vm_call_sget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                graphic_sprite(
                    call.get<int>(1),
                    call.get<int>(2)));
        }
        return 0;
    }

    int vm_call_sset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            graphic_sprite(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int vm_call_mget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                graphic_map(
                    call.get<int>(1),
                    call.get<int>(2)));
        }

        return 0;
    }

    int vm_call_mset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            graphic_map(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int vm_call_line(const VMCall &call) {
        if (call.is<int, int, int, int, uint8_t>()) {
            graphic_line(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5));
        }
        return 0;
    }

    int vm_call_circ(const VMCall &call) {
        if (call.is<int, int, int, uint8_t, bool>()) {
            graphic_circle(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<uint8_t>(4),
                call.get<bool>(5));
        }
        if (call.is<int, int, int, uint8_t>()) {
            graphic_circle(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<uint8_t>(4));
        }
        return 0;
    }

    int vm_call_rect(const VMCall &call) {
        if (call.is<int, int, int, int, uint8_t>()) {
            graphic_rect(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5));
        }

        if (call.is<int, int, int, int, uint8_t, bool>()) {
            graphic_rect(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5),
                call.get<bool>(6));
        }

        return 0;
    }

    int vm_call_print(const VMCall &call) {
        if (call.argc() < 3) {
            return 0;
        }

        auto st = call.is<std::string>(1) ? call.get<std::string>(1) : std::string();
        auto bx = call.is<int>(2) ? call.get<int>(2) : 0;
        auto by = call.is<int>(3) ? call.get<int>(3) : 0;
        auto co = call.is<uint8_t>(4) ? call.get<uint8_t>(4) : 1;
        auto w0 = call.is<int>(5) ? call.get<int>(5) : 4;
        auto w1 = call.is<int>(6) ? call.get<int>(6) : 8;

        w0 = std::clamp(w0, 1, 8);
        w1 = std::clamp(w1, 1, 8);

        if (!st.empty()) {
            auto x = bx;
            auto y = by;

            for (const auto &ch : st) {
                if (ch == '\n') {
                    y += 8;
                    x = bx;
                } else if (ch != '\r') {
                    graphic_char(ch, x, y, co, true);
                    x += (ch & 0x80) ? w1 : w0;
                }
            }
        }

        return 0;
    }

    int vm_call_spr(const VMCall &call) {
        if (call.argc() < 3) {
            return 0;
        }

        auto id = call.is<uint8_t>(1) ? call.get<uint8_t>(1) : 0;
        auto x = call.is<int>(2) ? call.get<int>(2) : 0;
        auto y = call.is<int>(3) ? call.get<int>(3) : 0;
        auto scale = call.is<int>(3) ? call.get<int>(3) : 1;
        auto flip = call.is<int>(4) ? call.get<int>(4) : 1;
        auto rotate = call.is<int>(5) ? call.get<int>(5) : 1;
        const auto pixelSize = std::clamp(scale, 1, 4);

        auto spX = (id & 0xF) << 3;
        auto spY = ((id >> 4) & 0xF) << 3;

        const auto flip_fn = [&](int &x, int &y) {
            if (flip & 0b1) {
                x = 7 - x;
            }
            if (flip & 0b10) {
                y = 7 - y;
            }
        };

        const auto rotate_fn = [&](int &x, int &y) {
            if (flip & 0b1) {
                auto tx = x;
                x = y;
                y = 7 - tx;
            }
            if (flip & 0b10) {
                x = 7 - x;
                y = 7 - y;
            }
        };

        for (auto inY = 0; inY < 8; inY++) {
            for (auto inX = 0; inX < 8; inX++) {
                auto tx = x;
                auto ty = y;
                auto color = graphic_sprite(spX + tx, spY + ty);

                flip_fn(tx, ty);
                rotate_fn(tx, ty);

                graphic_rect(
                    x + inX * pixelSize,
                    y + inY * pixelSize,
                    pixelSize,
                    pixelSize,
                    color,
                    true);
            }
        }

        return 0;
    }

    int vm_call_map(const VMCall &call) {
        if (call.argc() < 6) {
            return 0;
        }

        auto mx = call.is<int>(1) ? call.get<int>(1) : 0;
        auto my = call.is<int>(2) ? call.get<int>(2) : 0;
        auto mw = call.is<int>(3) ? call.get<int>(3) : 1;
        auto mh = call.is<int>(4) ? call.get<int>(4) : 1;
        auto sx = call.is<int>(5) ? call.get<int>(5) : 0;
        auto sy = call.is<int>(6) ? call.get<int>(6) : 0;
        auto scale = call.is<int>(7) ? call.get<int>(7) : 1;
        const auto layers = call.is<uint8_t>(8) ? call.get<int>(8) : 0xFF;

        scale = std::clamp(scale, 1, 4);

        const auto chunkSize = 8 * scale;
        const auto pixelSize = scale;

        if (mx < 0) {
            sx -= mx * chunkSize;
            mw += mx;
            mx = 0;
        }

        if (my < 0) {
            sy -= my * chunkSize;
            mh += my;
            my = 0;
        }

        if (mw < 0 || mh < 0)
            return 0;

        auto l = std::clamp(mx, 0, 128);
        auto t = std::clamp(my, 0, 128);
        auto r = std::clamp(mx + mw, 0, 128);
        auto b = std::clamp(my + mh, 0, 128);

        for (auto y = t; y < b; y++) {
            for (auto x = l; x < r; x++) {
                auto id = graphic_map(x, y);

                if (!(id & layers))
                    continue;

                auto spX = (id & 0xF) << 3;
                auto spY = ((id >> 4) & 0xF) << 3;

                for (auto inY = 0; inY < 8; inY++) {
                    for (auto inX = 0; inX < 8; inX++) {
                        auto color = graphic_sprite(spX + inX, spY + inY);
                        graphic_rect(
                            sx + x * chunkSize + inX * pixelSize,
                            sy + y * chunkSize + inY * pixelSize,
                            pixelSize,
                            pixelSize,
                            color,
                            true);
                    }
                }
            }
        }

        return 0;
    }

    int vm_call_btn(const VMCall &call) {
        if (call.is<uint8_t>()) {
            const auto i = call.get<uint8_t>(1);
            return call.ret(gamepad_down(i >> 3, 1 << (i & 0b111)));
        }

        return 0;
    }

    int vm_call_btnp(const VMCall &call) {
        if (call.is<uint8_t>()) {
            const auto i = call.get<uint8_t>(1);
            return call.ret(gamepad_pressed(i >> 3, 1 << (i & 0b111)));
        }

        return 0;
    }

    int vm_call_key(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(keybd_down(call.get<uint8_t>(1)));
        }
        return 0;
    }

    int vm_call_keyp(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(keybd_pressed(call.get<uint8_t>(1)));
        }
        return 0;
    }

    int vm_call_mouse(const VMCall &call) {
        return call.ret(mouse_x(), mouse_y(), mouse_z(), mouse_button());
    }

    int vm_call_log(const VMCall &call) {
        if (call.is<std::string>()) {
            signal_push(Signal::Print, call.get<std::string>(1));
        }
        return 0;
    }

    int vm_call_time(const VMCall &call) {
        return call.ret(state.timer.ticks());
    }

    int vm_call_tstamp(const VMCall &call) {
        auto unix_timestamp = std::chrono::seconds(std::time(NULL));
        return call.ret(unix_timestamp);
    }

    template <vm_call_fn Fn>
    static int vm_dispatcher(lua_State *l) {
        auto call = VMCall(l);
        return Fn(call);
    }

    template <vm_call_fn Fn>
    static void vm_bind(const char *fn_name) {
        lua_pushcclosure(state.vm, &vm_dispatcher<Fn>, 0);
        lua_setglobal(state.vm, fn_name);
    }

    bool vm_initialize(const std::string &src) {
        vm_release();

        state.vm = luaL_newstate();

        auto ret = luaL_loadstring(state.vm, src.data());

        if (ret) {
            state.err_msg = lua_tostring(state.vm, -1);
            return false;
        }

        luaL_openlibs(state.vm);

        lua_pcall(state.vm, 0, 0, 0);

        vm_bind<&vm_call_clip>("clip");
        vm_bind<&vm_call_pal>("pal");
        vm_bind<&vm_call_palt>("palt");
        vm_bind<&vm_call_cls>("cls");
        vm_bind<&vm_call_camera>("camera");
        vm_bind<&vm_call_pget>("pget");
        vm_bind<&vm_call_pset>("pset");
        vm_bind<&vm_call_fget>("fget");
        vm_bind<&vm_call_fset>("fset");
        vm_bind<&vm_call_sget>("sget");
        vm_bind<&vm_call_sset>("sset");
        vm_bind<&vm_call_mget>("mget");
        vm_bind<&vm_call_mset>("mset");
        vm_bind<&vm_call_line>("line");
        vm_bind<&vm_call_circ>("circ");
        vm_bind<&vm_call_rect>("rect");
        vm_bind<&vm_call_print>("print");
        vm_bind<&vm_call_spr>("spr");
        vm_bind<&vm_call_map>("map");
        vm_bind<&vm_call_btn>("btn");
        vm_bind<&vm_call_btnp>("btnp");
        vm_bind<&vm_call_key>("key");
        vm_bind<&vm_call_keyp>("keyp");
        vm_bind<&vm_call_mouse>("mouse");
        vm_bind<&vm_call_log>("log");
        vm_bind<&vm_call_time>("time");
        vm_bind<&vm_call_tstamp>("tstamp");
        vm_bind<&vm_call_pmem>("pmem");

        state.timer.reset();

        return true;
    }

    void vm_release() {
        if (state.vm) {
            lua_close(state.vm);
            state.vm = nullptr;
        }
    }

    const std::string &vm_error() {
        return state.err_msg;
    }

    bool vm_execute(const char *fn_name) {
        if (!state.vm)
            return false;

        auto p = lua_getglobal(state.vm, fn_name);
        auto i = lua_isfunction(state.vm, -1);

        if (!(p && i)) {
            lua_pop(state.vm, -1);
            return true;
        } else {
            auto ret = lua_pcall(state.vm, 0, 0, 0);
            if (ret) {
                state.err_msg = lua_tostring(state.vm, -1);
                return false;
            }
            return true;
        }
    }

}