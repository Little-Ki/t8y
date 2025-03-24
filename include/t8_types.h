#pragma once

#include <stdint.h>

namespace t8 {

    union bitfield_4  {
        uint8_t value;
        struct {
            uint8_t hi : 4;
            uint8_t lo : 4;
        };
    };

}