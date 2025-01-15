#include "timer.h"

namespace t8y {
    int Timer::ticks() {
        const auto now = std::chrono::system_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_started);
        const auto ms = duration.count();
        const auto ticks = static_cast<int>(ms / 15.625f);

        return ticks;
    }

    int Timer::steps() {
        return ticks() - m_passed;
    }

    void Timer::consume(int ticks) {
        m_passed += ticks;
    }

    void Timer::reset() {
        m_started = std::chrono::system_clock::now();
    }
}