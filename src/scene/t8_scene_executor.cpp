#include "t8_scene_executor.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_memory.h"
#include "t8_core_painter.h"
#include "t8_input_gamepad.h"
#include "t8_input_keybd.h"
#include "t8_input_mouse.h"

#include <algorithm>
#include <sol2/include/sol/sol.hpp>
#include <tuple>

using namespace t8::input;
using namespace t8::core;
using namespace t8::utils;

#define ASSERT_EXECUTE(x)                                       \
    {                                                           \
        auto r = (x);                                           \
        if (!r.valid()) {                                       \
            sol::error err = r;                                 \
            ctx_signals().push({SIGNAL_EXCEPTION, err.what()}); \
            return;                                             \
        }                                                       \
    }

namespace t8::scene {
    struct ExecutorState {
        uint8_t select;
        bool paused{false};
    };

    struct ScriptVM {
        sol::state lua;
        sol::protected_function init;
        sol::protected_function update;
        sol::protected_function draw;
    };

    static ExecutorState state;

    static std::optional<ScriptVM> vm;

    void setup_vm_api(sol::state &lua) {
        lua.set_function(
            "clip",
            sol::overload(
                [](int x, int y, int w, int h) { painter_clip(x, y, w, h); },
                []() { painter_clip(); }));

        lua.set_function(
            "pal",
            sol::overload(
                [](uint8_t a, uint8_t b) { painter_palette(a, b); },
                [](uint8_t a) { return painter_palette(a); }));

        lua.set_function(
            "palt",
            sol::overload(
                [](uint8_t c, bool b) { painter_palette_mask(c, b); },
                [](std::optional<uint16_t> m) { painter_palette_mask(m.value_or(0)); }));

        lua.set_function(
            "cls",
            [](std::optional<uint8_t> c) { painter_clear(c.value_or(0)); });

        lua.set_function(
            "pget",
            [](int x, int y) { return painter_pixel(x, y); });

        lua.set_function(
            "pset",
            [](int x, int y, uint8_t c) { painter_pixel(x, y, c); });

        lua.set_function(
            "fget",
            [](uint8_t i) { painter_flags(i); });

        lua.set_function(
            "fset",
            [](uint8_t i, uint8_t f) { painter_flags(i, f); });

        lua.set_function(
            "sget",
            [](int x, int y) { return painter_sprite(x, y); });

        lua.set_function(
            "sset",
            [](int x, int y, uint8_t c) { painter_sprite(x, y, c); });

        lua.set_function(
            "mget",
            [](int x, int y) { return painter_map(x, y); });

        lua.set_function(
            "mset",
            [](int x, int y, uint8_t i) { painter_map(x, y, i); });

        lua.set_function(
            "line",
            [](int x0, int y0, int x1, int y1, uint8_t c) { painter_line(x0, y0, x1, y1, c); });

        lua.set_function(
            "cric",
            [](int x, int y, int r, int c, std::optional<bool> f) {
                painter_circle(x, y, r, c, f.value_or(false));
            });

        lua.set_function(
            "rect",
            [](int x, int y, int w, int h, uint8_t c, std::optional<bool> f) {
                painter_rect(x, y, w, h, c, f.value_or(false));
            });

        lua.set_function(
            "print",
            [](
                std::string s, int x, int y,
                std::optional<int> _w, std::optional<int> _h,
                std::optional<uint8_t> _c) {
                const auto w = _w.value_or(4);
                const auto h = _h.value_or(8);
                const auto c = _c.value_or(1);
                const auto sx = x;
                for (auto &ch : s) {
                    if (ch == '\n') {
                        y += h;
                        x = sx;
                    } else if (ch != '\r') {
                        painter_char(ch, x, y, c, true);
                        x += w;
                    }
                }
            });

        lua.set_function(
            "btn",
            [](uint8_t i) { return gamepad_down(i >> 3, 1 << (i & 0b111)); });

        lua.set_function(
            "btnp",
            [](uint8_t i) { return gamepad_pressed(i >> 3, 1 << (i & 0b111)); });

        lua.set_function(
            "key",
            [](uint8_t k) { return keybd_down(k); });

        lua.set_function(
            "keyp",
            [](uint8_t k) { return keybd_pressed(k); });

        lua.set_function(
            "mouse",
            []() { return std::make_tuple(mouse_x(), mouse_y(), mouse_z(), mouse_button()); });

        lua.set_function(
            "log",
            [](std::string s) { ctx_signals().push({SIGNAL_PRINT, s}); });

        lua.set_function(
            "time",
            []() { return timer_ticks(); });

        lua.set_function(
            "tstamp",
            []() { return std::chrono::seconds(std::time(NULL)); });

        lua.set_function(
            "spr",
            [](uint8_t id, int x, int y, std::optional<int> _s, std::optional<int> _f, std::optional<int> _r) {
                const auto s = _s.value_or(0);
                const auto f = _f.value_or(0);
                const auto r = _r.value_or(0);
                const auto ps = std::clamp(s, 1, 4);

                auto sprite_x = (id & 0xF) << 3;
                auto sprite_y = ((id >> 4) & 0xF) << 3;

                for (auto dy = 0; dy < 8; dy++) {
                    for (auto dx = 0; dx < 8; dx++) {
                        auto tx = dx;
                        auto ty = dy;

                        if (f & 0b1)
                            tx = 7 - tx;

                        if (f & 0b10)
                            ty = 7 - ty;

                        if (r & 0b1) {
                            auto t = x;
                            tx = ty;
                            ty = 7 - t;
                        }

                        if (r & 0b10) {
                            tx = 7 - tx;
                            ty = 7 - ty;
                        }

                        auto color = painter_sprite(sprite_x + tx, sprite_y + ty);

                        painter_rect(x + dx * ps, y + dy * ps, ps, ps, color, true);
                    }
                }
            });

        lua.set_function(
            "map",
            [](int mx, int my, int mw, int mh, int sx, int sy, std::optional<int> _scale, std::optional<uint8_t> _layers) {
                const auto scale = std::clamp(_scale.value_or(1), 1, 4);
                const auto layers = _layers.value_or(0xff);
                const auto chunk_size = 8 * scale;
                const auto pixel_size = scale;

                if (mx < 0) {
                    sx -= mx * chunk_size;
                    mw += mx;
                    mx = 0;
                }

                if (my < 0) {
                    sy -= my * chunk_size;
                    mh += my;
                    my = 0;
                }

                if (mw < 0 || mh < 0)
                    return;

                auto l = std::clamp(mx, 0, 128);
                auto t = std::clamp(my, 0, 128);
                auto r = std::clamp(mx + mw, 0, 128);
                auto b = std::clamp(my + mh, 0, 128);

                for (auto y = t; y < b; y++) {
                    for (auto x = l; x < r; x++) {
                        auto id = painter_map(x, y);

                        if (!(id & layers))
                            continue;

                        auto sprite_x = (id & 0xF) << 3;
                        auto sprite_y = ((id >> 4) & 0xF) << 3;

                        for (auto dy = 0; dy < 8; dy++) {
                            for (auto dx = 0; dx < 8; dx++) {
                                auto color = painter_sprite(sprite_x + dx, sprite_y + dy);
                                painter_rect(
                                    sx + x * chunk_size + dx * pixel_size,
                                    sy + y * chunk_size + dy * pixel_size,
                                    pixel_size, pixel_size, color, true);
                            }
                        }
                    }
                }
            });
    }

    void executor_update() {
        if (keybd_pressed(41) && !state.paused) {
            state.paused = true;
            state.select = 0;
        }

        if (state.paused) {
            if (keybd_pressed(82) && state.select) {
                state.select -= 1;
            }
            if (keybd_pressed(81)) {
                state.select = std::clamp(state.select + 1, 0, 1);
            }
            if (keybd_pressed(40) || keybd_pressed(88)) {
                state.paused = false;
                if (state.select == 1) {
                    ctx_signals().push({SIGNAL_SWAP_CONSOLE});
                }
            }
        }

        if (!state.paused) {
            if (vm->update) {
                ASSERT_EXECUTE(vm->update());
            }
        }
    }

    void executor_draw() {
        painter_clear(0);

        if (vm->draw) {
            ASSERT_EXECUTE(vm->draw());
        }

        if (state.paused) {
            static const std::string menu[] = {"RESUME", "EXIT"};

            for (auto i = 0; i < 2; i++) {
                auto x = 1;
                auto y = 110 + 9 * i;

                for (const auto &ch : menu[i]) {
                    if (state.select == i) {
                        painter_char(ch, x, y, 3);
                    } else {
                        painter_char(ch, x, y, 1);
                    }
                    x += 4;
                }
            }
        }
    }

    void executor_enter() {
        swap_memory(true);
        
        vm.reset();
        vm.emplace();
        vm->lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
        setup_vm_api(vm->lua);

        ASSERT_EXECUTE(vm->lua.safe_script(ctx_script(), sol::script_pass_on_error));

        vm->init = vm->lua["init"];
        vm->update = vm->lua["update"];
        vm->draw = vm->lua["draw"];

        if (vm->init) {
            ASSERT_EXECUTE(vm->init());
        };

        painter_clear(0);

        ctx_signals().push({SIGNAL_STOP_INPUT});

        timer_reset();
    }

    void executor_leave() {
        swap_memory(false);
    }

}
