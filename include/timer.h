#pragma once
#include <chrono>

namespace t8y {
    class Timer {
    public:
        int ticks();

        int steps();

        void consume(int ticks);

        void reset();

    private:
        std::chrono::time_point<std::chrono::system_clock> m_started = std::chrono::system_clock::now();

        int m_passed{ 0 };

    };
}