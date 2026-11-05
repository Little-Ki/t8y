#include "core_emulator.h"
#include "core_context.h"
#include "core_gfx.h"
#include "core_window.h"
#include "core_memory.h"
#include "input_mouse.h"
#include "input_keyboard.h"
#include "input_gamepad.h"

#include "constants.h"

#include <thread>

using namespace t8::input;

#define RGBA(r, g, b, a) (((r && 0xff) << 24) | ((g && 0xff) << 16) | ((b && 0xff) << 8) | ((a && 0xff) << 0))

namespace t8::core
{
    bool Emulator::init()
    {
        if (!window::init(_wnd, 128, 128, _pixel_size))
        {
            return false;
        }

        gfx::reset_all(*_ctx.memory);
        setup_palette();
        setup_fonts();

        return true;
    }

    void Emulator::run()
    {

        SDL_Event event;

        while (true)
        {
            window::event(event);

            if (event.type != SDL_EVENT_QUIT)
            {
                handle_event(event);
            }
            else
            {
                return;
            }

            auto steps = _timer.steps();

            if (steps > 0)
            {
                scene_update();
                mouse::flush(_ctx.mouse);
                keyboard::flush(_ctx.keyboard);
                gamepad::flush(_ctx.gamepad);
            }

            _timer.consume(steps);

            scene_draw();

            auto p = _buffer;
            for (auto y = 0; y < 128; y++)
            {
                for (auto x = 0; x < 128; x++)
                {
                    auto n = gfx::get_pixel(*_ctx.memory, x, y);
                    n = gfx::get_pmap(*_ctx.memory, n);
                    *(p++) = _ctx.memory->palette[n];
                }
            }

            window::draw(_wnd, _buffer);

            handle_signal();
            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void Emulator::quit()
    {
        window::quit(_wnd);
    }

    void Emulator::scene_update()
    {
    }

    void Emulator::scene_draw()
    {
    }

    void Emulator::scene_enter()
    {
    }

    void Emulator::scene_leave()
    {
    }

    void Emulator::scene_swap(uint16_t next)
    {
    }

    void Emulator::setup_fonts()
    {
        std::memcpy(&_ctx.memory->default_font, FONT_DATA, 2048);
        std::memcpy(&_ctx.memory->custom_font, FONT_DATA, 2048);
    }

    void Emulator::setup_palette()
    {
        gfx::set_pcolor(*_ctx.memory, 0, RGBA(0, 0, 0, 255));
        gfx::set_pcolor(*_ctx.memory, 1, RGBA(250, 250, 250, 255));
        gfx::set_pcolor(*_ctx.memory, 2, RGBA(190, 190, 190, 255));
        gfx::set_pcolor(*_ctx.memory, 3, RGBA(200, 60, 90, 255));
        gfx::set_pcolor(*_ctx.memory, 4, RGBA(240, 130, 90, 255));
        gfx::set_pcolor(*_ctx.memory, 5, RGBA(250, 200, 120, 255));
        gfx::set_pcolor(*_ctx.memory, 6, RGBA(60, 180, 100, 255));
        gfx::set_pcolor(*_ctx.memory, 7, RGBA(40, 110, 120, 255));
        gfx::set_pcolor(*_ctx.memory, 8, RGBA(40, 70, 150, 255));
        gfx::set_pcolor(*_ctx.memory, 9, RGBA(150, 60, 150, 255));
        gfx::set_pcolor(*_ctx.memory, 10, RGBA(250, 120, 170, 255));
        gfx::set_pcolor(*_ctx.memory, 11, RGBA(250, 200, 170, 255));
        gfx::set_pcolor(*_ctx.memory, 12, RGBA(130, 140, 160, 255));
        gfx::set_pcolor(*_ctx.memory, 13, RGBA(150, 180, 190, 255));
        gfx::set_pcolor(*_ctx.memory, 14, RGBA(90, 110, 130, 255));
        gfx::set_pcolor(*_ctx.memory, 15, RGBA(50, 60, 90, 255));
    }

    void Emulator::handle_signal()
    {
        while (!_ctx.signals.empty())
        {
            const auto s = _ctx.signals.front();
            _ctx.signals.pop();

            if (s.type == SIGNAL_START_INPUT)
            {
                window::input(_wnd, true);
            }
            if (s.type == SIGNAL_STOP_INPUT)
            {
                window::input(_wnd, false);
            }
            if (s.type == SIGNAL_SWAP_EDITOR)
            {
                scene_swap(SCENE_ID_EDITOR);
            }
            if (s.type == SIGNAL_SWAP_CONSOLE)
            {
                scene_swap(SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_SWAP_EXECUTOR)
            {
                scene_swap(SCENE_ID_EXECUTOR);
            }
            if (s.type == SIGNAL_EXCEPTION)
            {
                scene_swap(SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_PRINT)
            {
            }
        }
    }

    void Emulator::handle_event(const SDL_Event &event)
    {

        switch (event.type)
        {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        {
            handle_mouse(event);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            handle_keybd(event);
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            _ctx.inputs.push(event.text.text);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        {
            handle_gamepad(event);
            break;
        }
        }
    }

    void Emulator::handle_mouse(const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            const auto &i = event.motion;
            mouse::move(
                _ctx.mouse,
                static_cast<int16_t>(i.x / _pixel_size),
                static_cast<int16_t>(i.y / _pixel_size));
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            const auto &i = event.button;
            mouse::button(_ctx.mouse, i.button, true);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            const auto &i = event.button;
            mouse::button(_ctx.mouse, i.button, false);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL)
        {
            const auto &i = event.wheel;
            mouse::wheel(_ctx.mouse, static_cast<int8_t>(i.y));
        }
    }

    void Emulator::handle_keybd(const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_KEY_DOWN ||
            event.type == SDL_EVENT_KEY_UP)
        {
            const auto &i = event.key;
            if (i.scancode > 255)
                return;

            keyboard::button(_ctx.keyboard, i.scancode, i.mod, i.repeat, event.type == SDL_EVENT_KEY_DOWN);
        }
    }

    void Emulator::handle_gamepad(const SDL_Event &event)
    {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED)
        {
            gamepad::join(_ctx.gamepad, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
        {
            gamepad::remove(_ctx.gamepad, event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            event.type == SDL_EVENT_GAMEPAD_BUTTON_UP)
        {
            const auto &i = event.gbutton;
            gamepad::button(_ctx.gamepad, i.which, i.button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
    }
}