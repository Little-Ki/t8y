#pragma once

#include "t8_timer.h"

namespace t8 {

    struct ExecutorState {
        uint8_t select;
        bool paused{false};
    };

    void executor_update();

    void executor_draw();

    void executor_enter();

    void executor_leave();

}