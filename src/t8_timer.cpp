#include "t8_timer.h"

namespace t8 {
    int Timer::ticks() {
        const auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - _started
        ).count();
        const auto ticks = static_cast<int>(delta * 0.064f);

        return ticks;
    }

    int Timer::steps() {
        return ticks() - _passed;
    }

    void Timer::consume(int ticks) {
        _passed += ticks;
    }

    void Timer::reset() {
        _started = std::chrono::system_clock::now();
    }
}