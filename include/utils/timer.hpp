#pragma once
#include <chrono>

namespace t8 {

    class Timer {
    public:
        Timer() {
            reset();
        }

        uint64_t ticks() {
            const auto now = std::chrono::steady_clock::now();
            const auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                                now - referece_time)
                                .count();
            return static_cast<uint64_t>((us * 4) / 62500);
        }

        uint64_t steps() {
            uint64_t current_ticks = ticks();
            if (current_ticks < tick_passed)
                return 0;
            return current_ticks - tick_passed;
        }

        void consume(uint64_t ticks) {
            tick_passed += ticks;
        }

        void reset() {
            referece_time = std::chrono::steady_clock::now();
            tick_passed = 0;
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> referece_time;
        uint64_t tick_passed;
    };
    
}