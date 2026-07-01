#include "core/window.h"

namespace t8::core
{

    bool window_init(WindowState &state, uint32_t width, uint32_t height, uint32_t pixel_size)
    {
        if (state.window)
        {
            return true;
        }

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        state.window = SDL_CreateWindow("T8Y", width * pixel_size, height * pixel_size, SDL_WINDOW_OPENGL);
        if (!state.window)
        {
            SDL_Log("Could not create a window: %s", SDL_GetError());
            return false;
        }

        SDL_SetWindowPosition(state.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        state.renderer = SDL_CreateRenderer(state.window, nullptr);

        if (!state.renderer)
        {
            SDL_Log("Create renderer failed: %s", SDL_GetError());
            return false;
        }

        state.texture = SDL_CreateTexture(
            state.renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            128, 128);

        if (!state.renderer)
        {
            SDL_Log("Create texture failed: %s", SDL_GetError());
            return false;
        }

        SDL_SetTextureScaleMode(state.texture, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);

        return true;
    }

    void window_quit(WindowState &state)
    {
        if (state.renderer)
        {
            SDL_DestroyRenderer(state.renderer);
            state.renderer = nullptr;
        }
        if (state.window)
        {
            SDL_DestroyWindow(state.window);
            state.window = nullptr;
        }
        SDL_Quit();
    }

    void window_draw(WindowState &state, const uint32_t *pixels)
    {
        SDL_RenderClear(state.renderer);
        SDL_UpdateTexture(state.texture, nullptr, pixels, 128 * sizeof(uint32_t));
        SDL_RenderTexture(state.renderer, state.texture, nullptr, nullptr);
        SDL_RenderPresent(state.renderer);
    }

    void window_event(SDL_Event &event)
    {
        SDL_PollEvent(&event);
    }

    void window_input(WindowState &state, bool enable)
    {
        if (enable)
        {
            SDL_StartTextInput(state.window);
        }
        else
        {
            SDL_StopTextInput(state.window);
        }
    }

}
