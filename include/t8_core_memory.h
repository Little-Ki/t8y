#pragma once

#include <stdint.h>

namespace t8::core {
    struct Memory {
        uint8_t screen[0x2000];
        uint8_t sprite[0x2000];

        uint8_t map[0x4000];

        uint32_t palette[0xF];
        uint8_t palette_mapping[0x8];
        uint16_t palette_mask;

        uint8_t flags[0x100];
        uint8_t default_font[0x800];
        uint8_t custom_font[0x800];

        uint8_t view_clip[4];
        int8_t draw_offset[2];

        uint32_t cache[256];
    };

    Memory *memory();

    void swap_memory(bool runtime);
}