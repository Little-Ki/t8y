#pragma once
#include <queue>
#include <string>
#include <variant>

#include "utils/timer.hpp"

namespace t8::input
{
    struct mouse::MouseState;
    struct keyboard::KeyboardState;
    struct gamepad::GamepadState;
}

namespace t8::scene
{
    struct console::ConsoleState;
}

namespace t8::core
{
    struct VirtualMemory;

    struct WindowState;

    struct Signal
    {
        uint32_t type;
        std::variant<
            std::monostate,
            std::string>
            value;
    };

    struct AppContext
    {
        VirtualMemory base_memory;
        VirtualMemory exec_memory;
        VirtualMemory *memory = &base_memory;

        std::string script;

        mouse::MouseState mouse;
        keyboard::KeyboardState keyboard;
        gamepad::GamepadState gamepad;
        window::WindowState window;

        std::queue<std::string> inputs;
        std::queue<Signal> signals;

        utils::Timer timer;

        uint32_t pixel_size = 3;
        uint32_t buffer[128 * 128];
    };
}