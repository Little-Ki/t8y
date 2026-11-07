#pragma once
#include <queue>
#include <string>
#include <variant>

#include "core_memory.h"
#include "input_mouse.h"
#include "input_gamepad.h"
#include "input_keyboard.h"
#include "input_keyboard.h"
#include "utils_timer.hpp"

namespace t8::core
{
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

        input::mouse::MouseState mouse;
        input::keyboard::KeyboardState keyboard;
        input::gamepad::GamepadState gamepad;

        std::queue<std::string> inputs;
        std::queue<Signal> signals;

        utils::Timer timer;
    };
}