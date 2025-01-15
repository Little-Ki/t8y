#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace t8y {

    class Window {
    public:
        Window() = default;

        bool initialize(uint32_t width, uint32_t height, float pixel_size);

        void quit();

        void draw(const std::vector<SDL_Vertex> &vertices, const std::vector<int> &indices);

        void pollEvent(SDL_Event &event);

        void textInput(bool enable);

    private:
        SDL_Window *m_window;

        SDL_Renderer *m_renderer;
        
    };
}