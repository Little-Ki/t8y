#include "t8_window.h"


namespace t8 {

    SDL_Window * window = nullptr;
    SDL_Renderer * renderer = nullptr;

    bool window_initialize(uint32_t width, uint32_t height, float pixel_size) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        window = SDL_CreateWindow("T8Y", width * pixel_size, height * pixel_size, 0);
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
    
    void window_draw(const std::vector<SDL_Vertex> &vertices, const std::vector<int> &indices) {
        SDL_RenderClear(renderer);
        SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size());
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
