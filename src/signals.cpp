#include "signals.h"

namespace t8y {
    void SignalQueue::join(int type, std::any value) {
        this->push(std::make_pair(type, value));
    }
}