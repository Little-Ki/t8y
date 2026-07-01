#include "core/emulator.h"
#include "core/context.h"
#include "core/painter.h"
#include "core/window.h"
#include "core/memory.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/gamepad.h"
#include "scene/console.h"

#include "constants.h"

#include <thread>

using namespace t8::input;

namespace t8::core
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
        painter_palette(mem, 0, 0, 0, 0, 255);
        painter_palette(mem, 1, 250, 250, 250, 255);
        painter_palette(mem, 2, 190, 190, 190, 255);
        painter_palette(mem, 3, 200, 60, 90, 255);
        painter_palette(mem, 4, 240, 130, 90, 255);
        painter_palette(mem, 5, 250, 200, 120, 255);
        painter_palette(mem, 6, 60, 180, 100, 255);
        painter_palette(mem, 7, 40, 110, 120, 255);
        painter_palette(mem, 8, 40, 70, 150, 255);
        painter_palette(mem, 9, 150, 60, 150, 255);
        painter_palette(mem, 10, 250, 120, 170, 255);
        painter_palette(mem, 11, 250, 200, 170, 255);
        painter_palette(mem, 12, 130, 140, 160, 255);
        painter_palette(mem, 13, 150, 180, 190, 255);
        painter_palette(mem, 14, 90, 110, 130, 255);
        painter_palette(mem, 15, 50, 60, 90, 255);
    }

    static void setup_fonts(VirtualMemory &memory)
    {
        std::memcpy(&memory.default_font, FONT_DATA, 2048);
        std::memcpy(&memory.custom_font, FONT_DATA, 2048);
    }

    bool emulator_init(AppContext &ctx)
    {
        if (!window_init(ctx.window_state, 128, 128, ctx.pixel_size))
        {
            return false;
        }

        painter_reset(*ctx.memory);
        setup_palette(*ctx.memory);
        setup_fonts(*ctx.memory);

        return true;
    }

    static void handle_mouse(MouseState &state, const SDL_Event &event, uint8_t pixel_size)
    {

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            const auto &i = event.motion;
            mouse_move(
                state,
                static_cast<int16_t>(i.x / pixel_size),
                static_cast<int16_t>(i.y / pixel_size));
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            const auto &i = event.button;
            mouse_button(state, i.button, true);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            const auto &i = event.button;
            mouse_button(state, i.button, false);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            const auto &i = event.wheel;
            mouse_wheel(state, static_cast<int8_t>(i.y));
        }
    }

    static void handle_keybd(KeyboardState &state, const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_KEY_DOWN ||
            event.type == SDL_EVENT_KEY_UP)
        {
            const auto &i = event.key;
            if (i.scancode > 255)
                return;

            keybd_button(state, i.scancode, i.mod, i.repeat, event.type == SDL_EVENT_KEY_DOWN);
        }
    }

    static void handle_gamepad(GamepadState &state, const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            gamepad_join(state, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            gamepad_remove(state, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
        {
            const auto &i = event.gbutton;
            gamepad_button(state, i.which, i.button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
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
            handle_mouse(ctx.mouse_state, event, ctx.pixel_size);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            handle_keybd(ctx.keybd_state, event);
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            ctx.input_queue.push(event.text.text);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        {
            handle_gamepad(ctx.gamepad_state, event);
            break;
        }
        }
    }

    static void handle_signal(AppContext &ctx)
    {

        while (!ctx.signal_queue.empty())
        {
            const auto s = ctx.signal_queue.front();
            ctx.signal_queue.pop();

            if (s.type == SIGNAL_START_INPUT)
            {
                window_input(ctx.window_state, true);
            }
            if (s.type == SIGNAL_STOP_INPUT)
            {
                window_input(ctx.window_state, false);
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
                console_print(std::get<std::string>(s.value), true);
                scene_swap(SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_PRINT)
            {
                console_print(std::get<std::string>(s.value), false);
            }
        }
    }

    void emulator_run(AppContext &ctx)
    {

        SDL_Event event;

        while (true)
        {
            window_event(event);

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
                mouse_flush(ctx.mouse_state);
                keybd_flush(ctx.keybd_state);
                gamepad_flush(ctx.gamepad_state);
            }

            timer->consume(steps);

            scene_draw(ctx);

            auto p = ctx.buffer;
            for (auto y = 0; y < 128; y++)
            {
                for (auto x = 0; x < 128; x++)
                {
                    auto n = painter_pixel(*ctx.memory, x, y);
                    n = painter_palette(*ctx.memory, n);
                    *(p++) = ctx.memory->palette[n];
                }
            }

            window_draw(ctx.window_state, ctx.buffer);

            handle_signal(ctx);
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void emulator_quit(AppContext &ctx)
    {
        window_quit(ctx.window_state);
    }
}