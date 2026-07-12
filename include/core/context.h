#pragma once
#include <queue>
#include <string>
#include <variant>

#include "core/memory.h"
#include "core/window.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "input/gamepad.h"

#include "utils/timer.hpp"

namespace t8::core {

    struct Signal {
        uint32_t type;
        std::variant<
            std::monostate,
            std::string>
            value;
    };

    struct AppContext {
        VirtualMemory base_memory;
        VirtualMemory exec_memory;
        VirtualMemory *memory = &base_memory;

        std::string script;

        input::MouseState mouse;
        input::KeyboardState keyboard;
        input::GamepadState gamepad;
        WindowState window;

        std::queue<std::string> inputs;
        std::queue<Signal> signals;

        utils::Timer timer;

        uint32_t pixel_size = 3;
        uint32_t buffer[128 * 128];
    };
}