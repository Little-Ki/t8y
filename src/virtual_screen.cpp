#include "virtual_screen.h"
#include <cassert>

namespace t8y {

    void VirtualScreen::initialize(int width, int height, int pixel_size) {
        assert(width > 0 && height > 0 && pixel_size > 0);

        m_width = width;
        m_height = height;
        m_vertices.resize(width * height * 4);
        m_indices.resize(width * height * 6);

        for (auto y = 0; y < height; y += 1) {
            for (auto x = 0; x < width; x += 1) {
                auto indexV = (y * width + x) * 4;
                auto indexI = (y * width + x) * 6;

                m_vertices[indexV + 0] = {static_cast<float>(x + 0) * pixel_size,
                                          static_cast<float>(y + 0) * pixel_size};
                m_vertices[indexV + 1] = {static_cast<float>(x + 1) * pixel_size,
                                          static_cast<float>(y + 0) * pixel_size};
                m_vertices[indexV + 2] = {static_cast<float>(x + 0) * pixel_size,
                                          static_cast<float>(y + 1) * pixel_size};
                m_vertices[indexV + 3] = {static_cast<float>(x + 1) * pixel_size,
                                          static_cast<float>(y + 1) * pixel_size};
                m_indices[indexI + 0] = indexV + 0;
                m_indices[indexI + 1] = indexV + 1;
                m_indices[indexI + 2] = indexV + 2;
                m_indices[indexI + 3] = indexV + 2;
                m_indices[indexI + 4] = indexV + 1;
                m_indices[indexI + 5] = indexV + 3;
            }
        }
    }

    void VirtualScreen::setPixel(int x, int y, uint8_t idx) {
        if (x < 0 || x >= m_width || y < 0 || idx & 0xF0)
            return;

        auto index = (y * m_width + x) * 4;
        if (index >= m_vertices.size())
            return;
        const SDL_FColor &color = m_palette[idx];

        m_vertices[index + 0].color = color;
        m_vertices[index + 1].color = color;
        m_vertices[index + 2].color = color;
        m_vertices[index + 3].color = color;
    }

    void VirtualScreen::setPalette(uint8_t idx, float r, float g, float b, float a) {
        if (idx & 0xF0)
            return;
        SDL_FColor &color = m_palette[idx];
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    void VirtualScreen::setPalette(uint8_t idx, int r, int g, int b, int a) {
        if (idx & 0xF0)
            return;

        SDL_FColor &color = m_palette[idx];
        color.r = r / 255.f;
        color.g = g / 255.f;
        color.b = b / 255.f;
        color.a = a / 255.f;
    }

    void VirtualScreen::clear(uint8_t idx) {
        if (idx & 0xF0)
            return;

        SDL_FColor &color = m_palette[idx];
        for (auto &v : m_vertices) {
            v.color = color;
        }
    }
    
}