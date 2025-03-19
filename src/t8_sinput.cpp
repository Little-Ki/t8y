#include "t8_sinput.h"

#include <queue>

namespace t8 {

    std::queue<std::string> queue;

    bool sinput_empty() {
        return queue.empty();
    }

    void sinput_push(const std::string &text) {
        queue.push(text);
    }

    void sinput_pop() {
        queue.pop();
    }

    const std::string &sinput_peek() {
        return queue.front();
    }

}