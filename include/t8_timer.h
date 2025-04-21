#pragma once
#include <chrono>

namespace t8 {

    struct TimerState {
        std::chrono::time_point<std::chrono::system_clock> referece_time = std::chrono::system_clock::now();
        int tick_passed;
    }

    int timer_ticks();

    int timer_steps();

    void timer_consume(int ticks);

    void timer_reset();

}