#include "t8_utils_timer.h"

namespace t8::utils {

    std::chrono::time_point<std::chrono::steady_clock> referece_time = std::chrono::steady_clock::now();
    uint64_t tick_passed;

    uint64_t timer_ticks() {
        const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now() - referece_time)
                               .count();
        const auto ticks = static_cast<uint64_t>(delta * 0.064f);

        return ticks;
    }

    uint64_t timer_steps() {
        return timer_ticks() - tick_passed;
    }

    void timer_consume(uint64_t ticks) {
        tick_passed += ticks;
    }

    void timer_reset() {
        referece_time = std::chrono::steady_clock::now();
    }

}