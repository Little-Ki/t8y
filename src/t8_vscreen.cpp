#include "t8_vscreen.h"
#include <array>
#include <cassert>

namespace t8 {

    int width, height;

    std::vector<SDL_Vertex> vertices;

    std::vector<int> indices;

    std::array<SDL_FColor, 16> palette;

    void vscreen_initialize(int _width, int _height, int pixel_size) {
        assert(_width > 0 && _height > 0 && pixel_size > 0);

        width = _width;
        height = _height;
        vertices.resize(width * height * 4);
        indices.resize(width * height * 6);

        for (auto y = 0; y < height; y += 1) {
            for (auto x = 0; x < width; x += 1) {
                auto i = (y * width + x) * 4;
                auto j = (y * width + x) * 6;

                vertices[i + 0] = {static_cast<float>(x + 0) * pixel_size,
                                   static_cast<float>(y + 0) * pixel_size};
                vertices[i + 1] = {static_cast<float>(x + 1) * pixel_size,
                                   static_cast<float>(y + 0) * pixel_size};
                vertices[i + 2] = {static_cast<float>(x + 0) * pixel_size,
                                   static_cast<float>(y + 1) * pixel_size};
                vertices[i + 3] = {static_cast<float>(x + 1) * pixel_size,
                                   static_cast<float>(y + 1) * pixel_size};
                indices[j + 0] = i + 0;
                indices[j + 1] = i + 1;
                indices[j + 2] = i + 2;
                indices[j + 3] = i + 2;
                indices[j + 4] = i + 1;
                indices[j + 5] = i + 3;
            }
        }

    }

    void vscreen_pixel(int x, int y, uint8_t id) {
        if (x < 0 || x >= width || y < 0 || id & 0xF0)
            return;

        auto index = (y * width + x) * 4;
        if (index >= vertices.size())
            return;

        const SDL_FColor &color = palette[id];

        vertices[index + 0].color = color;
        vertices[index + 1].color = color;
        vertices[index + 2].color = color;
        vertices[index + 3].color = color;
    }

    void vscreen_palette(uint8_t id, float r, float g, float b, float a) {
        if (id & 0xF0)
            return;
        SDL_FColor &color = palette[id];
        color.r = r;
        color.g = g;
        color.b = b;
        color.a = a;
    }

    void vscreen_palette(uint8_t id, int r, int g, int b, int a) {
        if (id & 0xF0)
            return;
        SDL_FColor &color = palette[id];
        color.r = r / 255.f;
        color.g = g / 255.f;
        color.b = b / 255.f;
        color.a = a / 255.f;
    }

    void vscreen_clear(uint8_t id) {

        if (id & 0xF0)
            return;

        SDL_FColor &color = palette[id];

        for (auto &v : vertices) {
            v.color = color;
        }
    }

    const std::vector<SDL_Vertex> &vscreen_vertices() {
        return vertices;
    }

    const std::vector<int> &vscreen_indices() {
        return indices;
    }

}
