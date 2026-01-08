#pragma once
#include <functional>
#include <unordered_map>

namespace t8::core {
    bool emulator_initialize();

    void emulator_run();

    void emulator_quit();
}