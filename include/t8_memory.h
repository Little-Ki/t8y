#pragma once

#include <stdint.h>

namespace t8 {
    
    struct Memory {
        uint8_t screen[0x2000];
        uint8_t sprite[0x2000];
        uint8_t map[0x4000];
        uint8_t palette[0x8];
        uint16_t opacity;
        uint8_t flags[0x100];
        uint8_t default_font[0x800];
        uint8_t custom_font[0x800];

        uint8_t view_clip[4];
        int8_t draw_offset[2];
    };

    Memory *mem();

}