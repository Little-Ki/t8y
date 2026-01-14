#pragma once
#include <chrono>

namespace t8::utils {
    uint64_t timer_ticks();

    uint64_t timer_steps();

    void timer_consume(uint64_t ticks);

    void timer_reset();
}