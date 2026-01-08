#pragma once

#include "t8_utils_timer.h"

namespace t8::scene {
    struct ExecutorState {
        uint8_t select;
        bool paused{false};
    };

    void executor_update();

    void executor_draw();

    void executor_enter();

    void executor_leave();
}