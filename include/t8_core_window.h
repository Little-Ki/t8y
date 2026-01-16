#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace t8::core {
    bool window_initialize(uint32_t width, uint32_t height, uint32_t pixel_size);

    void window_quit();

    void window_draw(const uint32_t *pixels);

    void window_event(SDL_Event &event);

    void window_input(bool enable);
}