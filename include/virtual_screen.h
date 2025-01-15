#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <array>

namespace t8y {

    class VirtualScreen {
    public:
        void initialize(int width, int height, int pixel_size);

        void setPixel(int x, int y, uint8_t idx);

        void setPalette(uint8_t idx, float r = 0, float g = 0, float b = 0, float a = 1);

        void setPalette(uint8_t idx, int r = 0, int g = 0, int b = 0, int a = 0xFF);

        void clear(uint8_t idx);
        
        const auto &vertices() {
            return m_vertices;
        };

        const auto &indices() {
            return m_indices;
        };

    private:
        int m_width, m_height;

        std::vector<SDL_Vertex> m_vertices;
        
        std::vector<int> m_indices;

        std::array<SDL_FColor, 16> m_palette;

    };
    
}