#include "t8_signal.h"

#include <queue>

namespace t8 {

    std::queue<signal_t> queue;

    bool signal_empty() {
        return queue.empty();
    }

    void signal_push(Signal type, std::any value) {
        queue.push(std::make_pair(type, value));
    }

    void signal_pop() {
        queue.pop();
    }

    const signal_t &signal_peek() {
        return queue.front();
    }

}