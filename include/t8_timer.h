#pragma once
#include <chrono>

namespace t8 {

    class Timer {
    public:
        int ticks();

        int steps();

        void consume(int ticks);

        void reset();

    private:
        std::chrono::time_point<std::chrono::system_clock> _started = std::chrono::system_clock::now();

        int _passed{0};
    };
    
}