#include "core/emulator.h"
#include "core/context.h"
#include "core/gfx.h"
#include "core/window.h"
#include "core/memory.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/gamepad.h"

#include "constants.h"

#include <thread>

using namespace t8::input;
using namespace t8::scene;

#define RGBA(r, g, b, a) (((r && 0xff) << 24) | ((g && 0xff) << 16) | ((b && 0xff) << 8) | ((a && 0xff) << 0))

namespace t8::core::emulator
{
    static void scene_update(AppContext &ctx)
    {
    }

    static void scene_draw(AppContext &ctx)
    {
    }

    static void scene_enter(AppContext &ctx)
    {
    }

    static void scene_leave(AppContext &ctx)
    {
    }

    static void scene_swap(AppContext &ctx, uint16_t next)
    {
    }

    static void setup_palette(VirtualMemory &mem)
    {
        gfx::set_pcolor(mem, 0, RGBA(0, 0, 0, 255));
        gfx::set_pcolor(mem, 1, RGBA(250, 250, 250, 255));
        gfx::set_pcolor(mem, 2, RGBA(190, 190, 190, 255));
        gfx::set_pcolor(mem, 3, RGBA(200, 60, 90, 255));
        gfx::set_pcolor(mem, 4, RGBA(240, 130, 90, 255));
        gfx::set_pcolor(mem, 5, RGBA(250, 200, 120, 255));
        gfx::set_pcolor(mem, 6, RGBA(60, 180, 100, 255));
        gfx::set_pcolor(mem, 7, RGBA(40, 110, 120, 255));
        gfx::set_pcolor(mem, 8, RGBA(40, 70, 150, 255));
        gfx::set_pcolor(mem, 9, RGBA(150, 60, 150, 255));
        gfx::set_pcolor(mem, 10, RGBA(250, 120, 170, 255));
        gfx::set_pcolor(mem, 11, RGBA(250, 200, 170, 255));
        gfx::set_pcolor(mem, 12, RGBA(130, 140, 160, 255));
        gfx::set_pcolor(mem, 13, RGBA(150, 180, 190, 255));
        gfx::set_pcolor(mem, 14, RGBA(90, 110, 130, 255));
        gfx::set_pcolor(mem, 15, RGBA(50, 60, 90, 255));
    }

    static void setup_fonts(VirtualMemory &mem)
    {
        std::memcpy(&mem.default_font, FONT_DATA, 2048);
        std::memcpy(&mem.custom_font, FONT_DATA, 2048);
    }

    bool init(AppContext &ctx)
    {
        if (!window::init(ctx.window, 128, 128, ctx.pixel_size))
        {
            return false;
        }

        gfx::reset_all(*ctx.memory);
        setup_palette(*ctx.memory);
        setup_fonts(*ctx.memory);

        return true;
    }

    static void handle_mouse(mouse::MouseState &state, const SDL_Event &event, uint8_t pixel_size)
    {

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            const auto &i = event.motion;
            mouse::move(
                state,
                static_cast<int16_t>(i.x / pixel_size),
                static_cast<int16_t>(i.y / pixel_size));
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            const auto &i = event.button;
            mouse::button(state, i.button, true);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            const auto &i = event.button;
            mouse::button(state, i.button, false);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            const auto &i = event.wheel;
            mouse::wheel(state, static_cast<int8_t>(i.y));
        }
    }

    static void handle_keybd(keyboard::KeyboardState &state, const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_KEY_DOWN ||
            event.type == SDL_EVENT_KEY_UP)
        {
            const auto &i = event.key;
            if (i.scancode > 255)
                return;

            keyboard::button(state, i.scancode, i.mod, i.repeat, event.type == SDL_EVENT_KEY_DOWN);
        }
    }

    static void handle_gamepad(gamepad::GamepadState &state, const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            gamepad::join(state, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            gamepad::remove(state, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
        {
            const auto &i = event.gbutton;
            gamepad::button(state, i.which, i.button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
    }

    static void handle_event(AppContext &ctx, const SDL_Event &event)
    {
        switch (event.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        {
            handle_mouse(ctx.mouse, event, ctx.pixel_size);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            handle_keybd(ctx.keyboard, event);
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            ctx.inputs.push(event.text.text);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        {
            handle_gamepad(ctx.gamepad, event);
            break;
        }
        }
    }

    static void handle_signal(AppContext &ctx)
    {

        while (!ctx.signals.empty())
        {
            const auto s = ctx.signals.front();
            ctx.signals.pop();

            if (s.type == SIGNAL_START_INPUT)
            {
                window::input(ctx.window, true);
            }
            if (s.type == SIGNAL_STOP_INPUT)
            {
                window::input(ctx.window, false);
            }
            if (s.type == SIGNAL_SWAP_EDITOR)
            {
                scene_swap(ctx, SCENE_ID_EDITOR);
            }
            if (s.type == SIGNAL_SWAP_CONSOLE)
            {
                scene_swap(ctx, SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_SWAP_EXECUTOR)
            {
                scene_swap(ctx, SCENE_ID_EXECUTOR);
            }
            if (s.type == SIGNAL_EXCEPTION)
            {
                scene_swap(ctx, SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_PRINT)
            {
                        }
        }
    }

    void run(AppContext &ctx)
    {

        SDL_Event event;

        while (true)
        {
            window::event(event);

            if (event.type != SDL_EVENT_QUIT)
            {
                handle_event(ctx, event);
            }
            else
            {
                return;
            }

            auto timer = &ctx.timer;
            auto steps = timer->steps();

            if (steps > 0)
            {
                scene_update(ctx);
                mouse::flush(ctx.mouse);
                keyboard::flush(ctx.keyboard);
                gamepad::flush(ctx.gamepad);
            }

            timer->consume(steps);

            scene_draw(ctx);

            auto p = ctx.buffer;
            for (auto y = 0; y < 128; y++)
            {
                for (auto x = 0; x < 128; x++)
                {
                    auto n = gfx::get_pixel(*ctx.memory, x, y);
                    n = gfx::get_pmap(*ctx.memory, n);
                    *(p++) = ctx.memory->palette[n];
                }
            }

            window::draw(ctx.window, ctx.buffer);

            handle_signal(ctx);
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void quit(AppContext &ctx)
    {
        window::quit(ctx.window);
    }
}