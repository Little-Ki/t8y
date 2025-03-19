#pragma once
#include <any>
#include <queue>
#include <tuple>

namespace t8y {

#define SIGNAL_SWAP_EDIT 0
#define SIGNAL_SWAP_CONSOLE 1
#define SIGNAL_SWAP_EXEC 2
#define SIGNAL_START_INPUT 3
#define SIGNAL_STOP_INPUT 4
#define SIGNAL_EXCEPTION 5
#define SIGNAL_OUTPUT 6

    class SignalQueue : public std::queue<std::pair<int, std::any>> {
    public:
        void join(int type, std::any value = {});
    };

}