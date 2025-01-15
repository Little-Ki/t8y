#include "window.h"

namespace t8y {

    bool Window::initialize(uint32_t width, uint32_t height, float pixel_size) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_Log("SDL_Init failed: %s", SDL_GetError());
            return false;
        }

        m_window = SDL_CreateWindow("T8Y", width * pixel_size, height * pixel_size, 0);
        if (!m_window) {
            SDL_Log("Could not create a window: %s", SDL_GetError());
            return false;
        }

        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

        m_renderer = SDL_CreateRenderer(m_window, nullptr);

        if (!m_renderer) {
            SDL_Log("Create renderer failed: %s", SDL_GetError());
            return false;
        }

        return true;
    }

    void Window::quit() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        SDL_Quit();
    }

    void Window::draw(const std::vector<SDL_Vertex> &vertices, const std::vector<int> &indices) {
        SDL_RenderClear(m_renderer);
        SDL_RenderGeometry(m_renderer, nullptr, vertices.data(), vertices.size(), indices.data(), indices.size());
        SDL_RenderPresent(m_renderer);
    }

    void Window::pollEvent(SDL_Event &event) {
        SDL_PollEvent(&event);
    }

    void Window::textInput(bool enable) {
        if (enable) {
            SDL_StartTextInput(m_window);
        } else {
            SDL_StopTextInput(m_window);
        }
    }
}