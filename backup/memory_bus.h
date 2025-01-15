#pragma once

#include <memory>
#include "types.h"

namespace t8y {

    class MemoryBus {
    public:
        MemoryBus(std::shared_ptr<AppContext> ctx);

        template <typename T>
        T read(uintptr_t addr) {
            union {
                uint8_t bytes[sizeof(T)];
                T value = T();
            } u;

            for (auto i = 0; i < sizeof(T); i++) {
                auto ptr = address(addr + i);

                if (ptr)
                    u.bytes[i] = *ptr;
            }

            return u.value;
        }

        template <typename T>
        void write(uintptr_t addr, T value) {
            union {
                uint8_t bytes[sizeof(T)];
                T value;
            } u;
            u.value = value;

            for (auto i = 0; i < sizeof(T); i++) {
                write(addr + i, u.bytes[i]);
            }
        }

        template <typename T = uint8_t>
        void write(uintptr_t addr, uint8_t value) {
            auto ptr = address(addr);

            if (ptr)
                *ptr = value;
        }

    private:
        uint8_t *address(uintptr_t addr);

        std::shared_ptr<AppContext> ctx;

    };

}