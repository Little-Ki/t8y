#pragma once
#include <cstdint>

#define RGBA(r, g, b, a) (((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff))

namespace t8 {
    constexpr uint32_t SCENE_ID_CONSOLE = 0;
    constexpr uint32_t SCENE_ID_EDITOR = 1;
    constexpr uint32_t SCENE_ID_EXECUTOR = 2;

    constexpr uint32_t SIGNAL_SWAP_EDITOR = 0;
    constexpr uint32_t SIGNAL_SWAP_CONSOLE = 1;
    constexpr uint32_t SIGNAL_SWAP_EXECUTOR = 2;
    constexpr uint32_t SIGNAL_START_INPUT = 3;
    constexpr uint32_t SIGNAL_STOP_INPUT = 4;
    constexpr uint32_t SIGNAL_PRINT = 5;
    constexpr uint32_t SIGNAL_EXCEPTION = 5;
}