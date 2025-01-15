#include "memory_bus.h"

#include "graphic.h"
#include "input.h"

namespace t8y {
    MemoryBus::MemoryBus(std::shared_ptr<AppContext> ctx) : ctx(ctx) {
    }

    uint8_t *MemoryBus::address(uintptr_t addr) {
        return nullptr;
    }
}
