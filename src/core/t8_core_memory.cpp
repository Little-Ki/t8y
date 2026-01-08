#include "t8_core_memory.h"
#include <memory>

namespace t8::core {
    static Memory base_memory{};
    static Memory runtime_memory{};
    static Memory *current_memory = &base_memory;

    Memory *memory() {
        return current_memory;
    }

    void swap_memory(bool runtime) {
        current_memory = runtime ? &runtime_memory : &base_memory;

        if (runtime) {
            std::memcpy(&runtime_memory, &base_memory, sizeof(Memory));
        }
    }
}