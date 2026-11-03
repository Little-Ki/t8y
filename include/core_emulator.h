#pragma once
#include <functional>
#include <unordered_map>

#include "core_context.h"
#include "core_window.h"
#include "utils_timer.hpp"

namespace t8::core
{
    class Emulator
    {
    public:
        bool init();
        void run();
        void quit();

    private:
        void scene_update();
        void scene_draw();
        void scene_enter();
        void scene_leave();
        void scene_swap(uint16_t next);

        void setup_fonts();
        void setup_palette();

        void handle_signal();

        void handle_event(const SDL_Event &event);
        void handle_mouse(const SDL_Event &event);
        void handle_keybd(const SDL_Event &event);
        void handle_gamepad(const SDL_Event &event);

    private:
        AppContext _ctx;
        window::WindowState _wnd;

        utils::Timer _timer;
        uint32_t _pixel_size = 3;
        uint32_t _buffer[128 * 128];
    };
}