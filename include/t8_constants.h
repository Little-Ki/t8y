#pragma once
#include <cstdint>
#include <SDL3/SDL_scancode.h>

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

    constexpr auto SCANCODE_ESC = SDL_SCANCODE_ESCAPE;
    constexpr auto SCANCODE_LEFT = SDL_SCANCODE_LEFT;
    constexpr auto SCANCODE_UP = SDL_SCANCODE_UP;
    constexpr auto SCANCODE_RIGHT = SDL_SCANCODE_RIGHT;
    constexpr auto SCANCODE_DOWN = SDL_SCANCODE_DOWN;
    constexpr auto SCANCODE_RETURN = SDL_SCANCODE_RETURN;
    constexpr auto SCANCODE_ENTER = SDL_SCANCODE_KP_ENTER;
    constexpr auto SCANCODE_BACKSPACE = SDL_SCANCODE_BACKSPACE;
    constexpr auto SCANCODE_DELETE = SDL_SCANCODE_DELETE;
    constexpr auto SCANCODE_TAB = SDL_SCANCODE_TAB;
    constexpr auto SCANCODE_A = SDL_SCANCODE_A;
    constexpr auto SCANCODE_R = SDL_SCANCODE_R;
    constexpr auto SCANCODE_Z = SDL_SCANCODE_Z;
}