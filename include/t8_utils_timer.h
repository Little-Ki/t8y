#pragma once
#include <chrono>

namespace t8::utils {
    int timer_ticks();

    int timer_steps();

    void timer_consume(int ticks);

    void timer_reset();
}