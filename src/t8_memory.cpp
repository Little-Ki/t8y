#include "t8_memory.h"

namespace t8 {
    
    Memory _mem{};

    Memory *mem() {
        return &_mem;
    }

}