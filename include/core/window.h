#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace t8::core::window
{
    struct WindowState
    {
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        SDL_Texture *texture = nullptr;
    };

    bool init(WindowState &state, uint32_t width, uint32_t height, uint32_t pixel_size);

    void quit(WindowState &state);

    void draw(WindowState &state, const uint32_t *pixels);

    void event(SDL_Event &event);

    void input(WindowState &state, bool enable);
}