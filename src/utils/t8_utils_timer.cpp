#include "t8_utils_timer.h"

namespace t8::utils {

    std::chrono::time_point<std::chrono::system_clock> referece_time = std::chrono::system_clock::now();
    int tick_passed;

    int timer_ticks() {
        const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::system_clock::now() - referece_time)
                               .count();
        const auto ticks = static_cast<int>(delta * 0.064f);

        return ticks;
    }

    int timer_steps() {
        return timer_ticks() - tick_passed;
    }

    void timer_consume(int ticks) {
        tick_passed += ticks;
    }

    void timer_reset() {
        referece_time = std::chrono::system_clock::now();
    }

}