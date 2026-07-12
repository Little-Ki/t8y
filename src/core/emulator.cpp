#include "core/emulator.h"
#include "core/context.h"
#include "core/gfx.h"
#include "core/memory.h"
#include "core/window.h"
#include "input/gamepad.h"
#include "input/keyboard.h"
#include "input/mouse.h"

#include "constants.h"

#include <thread>

using namespace t8::input;
using namespace t8::core;

#define _RGBA(r, g, b, a) (((r && 0xff) << 24) | ((g && 0xff) << 16) | ((b && 0xff) << 8) | ((a && 0xff) << 0))

namespace t8::core {
    static void scene_update(AppContext &ctx) {
    }

    static void scene_draw(AppContext &ctx) {
    }

    static void scene_enter(AppContext &ctx) {
    }

    static void scene_leave(AppContext &ctx) {
    }

    static void scene_swap(AppContext &ctx, uint16_t next) {
    }

    bool emu_init(AppContext &ctx) {
        auto mem = *ctx.memory;

        if (!wnd_init(ctx.window, 128, 128, ctx.pixel_size)) {
            return false;
        }

        gfx_set_pcolor(mem, 0, _RGBA(0, 0, 0, 255));
        gfx_set_pcolor(mem, 1, _RGBA(250, 250, 250, 255));
        gfx_set_pcolor(mem, 2, _RGBA(190, 190, 190, 255));
        gfx_set_pcolor(mem, 3, _RGBA(200, 60, 90, 255));
        gfx_set_pcolor(mem, 4, _RGBA(240, 130, 90, 255));
        gfx_set_pcolor(mem, 5, _RGBA(250, 200, 120, 255));
        gfx_set_pcolor(mem, 6, _RGBA(60, 180, 100, 255));
        gfx_set_pcolor(mem, 7, _RGBA(40, 110, 120, 255));
        gfx_set_pcolor(mem, 8, _RGBA(40, 70, 150, 255));
        gfx_set_pcolor(mem, 9, _RGBA(150, 60, 150, 255));
        gfx_set_pcolor(mem, 10, _RGBA(250, 120, 170, 255));
        gfx_set_pcolor(mem, 11, _RGBA(250, 200, 170, 255));
        gfx_set_pcolor(mem, 12, _RGBA(130, 140, 160, 255));
        gfx_set_pcolor(mem, 13, _RGBA(150, 180, 190, 255));
        gfx_set_pcolor(mem, 14, _RGBA(90, 110, 130, 255));
        gfx_set_pcolor(mem, 15, _RGBA(50, 60, 90, 255));
        std::memcpy(&mem.default_font, FONT_DATA, 2048);
        std::memcpy(&mem.custom_font, FONT_DATA, 2048);

        return true;
    }

    static void on_mouse(MouseState &s, const SDL_Event &e, uint8_t pixel_size) {
        if (e.type == SDL_EVENT_MOUSE_MOTION) {
            const auto &i = e.motion;
            m_move(
                s,
                static_cast<int16_t>(i.x / pixel_size),
                static_cast<int16_t>(i.y / pixel_size));
        }

        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            const auto &i = e.button;
            m_button(s, i.button, true);
        }

        if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            const auto &i = e.button;
            m_button(s, i.button, false);
        }

        if (e.type == SDL_EVENT_MOUSE_WHEEL) {
            const auto &i = e.wheel;
            m_wheel(s, static_cast<int8_t>(i.y));
        }
    }

    static void on_keybd(KeyboardState &s, const SDL_Event &e) {
        if (e.type == SDL_EVENT_KEY_DOWN ||
            e.type == SDL_EVENT_KEY_UP) {
            const auto &i = e.key;
            if (i.scancode > 255)
                return;

            k_button(s, i.scancode, i.mod, i.repeat, e.type == SDL_EVENT_KEY_DOWN);
        }
    }

    static void on_gamepad(GamepadState &s, const SDL_Event &e) {
        if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
            g_join(s, e.gdevice.which);
        }

        if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
            g_remove(s, e.gdevice.which);
        }

        if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            e.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            const auto &i = e.gbutton;
            g_button(s, i.which, i.button, e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
    }

    static void on_event(AppContext &ctx, const SDL_Event &e) {
        switch (e.type) {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL: {
            on_mouse(ctx.mouse, e, ctx.pixel_size);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            on_keybd(ctx.keyboard, e);
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            ctx.inputs.push(e.text.text);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            on_gamepad(ctx.gamepad, e);
            break;
        }
        }
    }

    static void on_signal(AppContext &ctx) {
        while (!ctx.signals.empty()) {
            const auto s = ctx.signals.front();
            ctx.signals.pop();

            if (s.type == SIGNAL_START_INPUT) {
                wnd_input(ctx.window, true);
            }
            if (s.type == SIGNAL_STOP_INPUT) {
                wnd_input(ctx.window, false);
            }
            if (s.type == SIGNAL_SWAP_EDITOR) {
                scene_swap(ctx, SCENE_ID_EDITOR);
            }
            if (s.type == SIGNAL_SWAP_CONSOLE) {
                scene_swap(ctx, SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_SWAP_EXECUTOR) {
                scene_swap(ctx, SCENE_ID_EXECUTOR);
            }
            if (s.type == SIGNAL_EXCEPTION) {
                scene_swap(ctx, SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_PRINT) {
            }
        }
    }

    void emu_run(AppContext &ctx) {

        SDL_Event e;

        while (true) {

            while (wnd_event(e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    return;
                }
                on_event(ctx, e);
            }

            auto timer = &ctx.timer;
            auto steps = timer->steps();

            if (steps > 0) {
                scene_update(ctx);
                m_flush(ctx.mouse);
                k_flush(ctx.keyboard);
                g_flush(ctx.gamepad);
            }

            timer->consume(steps);

            scene_draw(ctx);

            auto p = ctx.buffer;
            for (auto y = 0; y < 128; y++) {
                for (auto x = 0; x < 128; x++) {
                    auto n = gfx_get_pixel(*ctx.memory, x, y);
                    n = gfx_get_pmap(*ctx.memory, n);
                    *(p++) = ctx.memory->palette[n];
                }
            }

            wnd_draw(ctx.window, ctx.buffer);

            on_signal(ctx);
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void emu_quit(AppContext &ctx) {
        wnd_quit(ctx.window);
    }
}