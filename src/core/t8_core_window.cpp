#include "t8_core_window.h"

namespace t8::core {

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    bool window_initialize(uint32_t width, uint32_t height, float pixel_size) {
        if (window) {
            return true;
        }

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        window = SDL_CreateWindow("T8Y", width * pixel_size, height * pixel_size, SDL_WINDOW_OPENGL);
        if (!window) {
            SDL_Log("Could not create a window: %s", SDL_GetError());
            return false;
        }

        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        renderer = SDL_CreateRenderer(window, nullptr);

        if (!renderer) {
            SDL_Log("Create renderer failed: %s", SDL_GetError());
            return false;
        }

        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            128, 128);

        if (!renderer) {
            SDL_Log("Create texture failed: %s", SDL_GetError());
            return false;
        }

        SDL_SetTextureScaleMode(texture, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);
        
        return true;
    }

    void window_quit() {
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        SDL_Quit();
    }

    void window_draw(const uint32_t *pixels) {
        SDL_RenderClear(renderer);
        SDL_UpdateTexture(texture, nullptr, pixels, 128 * sizeof(uint32_t));
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void window_event(SDL_Event &event) {
        SDL_PollEvent(&event);
    }

    void window_input(bool enable) {
        if (enable) {
            SDL_StartTextInput(window);
        } else {
            SDL_StopTextInput(window);
        }
    }

}
