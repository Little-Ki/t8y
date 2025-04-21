#include "t8_timer.h"

namespace t8 {

    TimerState state;

    int timer_ticks() {
        const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - state.referece_time
        ).count();
        const auto ticks = static_cast<int>(delta * 0.064f);

        return ticks;
    }

    int timer_steps() {
        return timer_ticks() - state.tick_passed;
    }

    void timer_consume(int ticks) {
        state.tick_passed += ticks;
    }

    void timer_reset() {
        state.referece_time = std::chrono::system_clock::now();
    }
    
}