#pragma once
#include <SDL3/SDL.h>
#include <vector>

namespace t8 {

    void vscreen_initialize(int width, int height, int pixel_size);

    void vscreen_pixel(int x, int y, uint8_t id);

    void vscreen_palette(uint8_t id, float r = 0, float g = 0, float b = 0, float a = 1);

    void vscreen_palette(uint8_t id, int r = 0, int g = 0, int b = 0, int a = 0xFF);

    void vscreen_clear(uint8_t id);
    
    const std::vector<SDL_Vertex> &vscreen_vertices();

    const std::vector<int> &vscreen_indices() ;

}