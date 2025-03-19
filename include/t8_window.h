#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace t8 {

    bool window_initialize(uint32_t width, uint32_t height, float pixel_size);

    void window_quit();

    void window_draw(const std::vector<SDL_Vertex> &vertices, const std::vector<int> &indices);

    void window_event(SDL_Event &event);

    void window_input(bool enable);
    
}