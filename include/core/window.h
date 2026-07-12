#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace t8::core
{
    struct WindowState
    {
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        SDL_Texture *texture = nullptr;
    };

    bool wnd_init(WindowState &state, uint32_t width, uint32_t height, uint32_t pixel_size);

    void wnd_quit(WindowState &state);

    void wnd_draw(WindowState &state, const uint32_t *pixels);

    bool wnd_event(SDL_Event &event);

    void wnd_input(WindowState &state, bool enable);
}