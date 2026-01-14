#include "t8_scene_executor.h"
#include "t8_constants.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_core_vm.h"
#include "t8_input_keyboard.h"

#include <algorithm>

using namespace t8::input;
using namespace t8::core;
using namespace t8::utils;

namespace t8::scene {

    ExecutorState state;

    void executor_update() {
        if (keyboard_pressed(41) && !state.paused) {
            state.paused = true;
            state.select = 0;
        }

        if (state.paused) {
            if (keyboard_pressed(82) && state.select) {
                state.select -= 1;
            }
            if (keyboard_pressed(81)) {
                state.select = std::clamp(state.select + 1, 0, 1);
            }
            if (keyboard_pressed(40) || keyboard_pressed(88)) {
                state.paused = false;
                if (state.select == 1) {
                    context()->signals.push({SIGNAL_SWAP_CONSOLE});
                }
            }
        }

        auto steps = timer_steps();
        if (steps < 3 && !state.paused) {
            for (auto i = 0; i < steps; i++) {
                if (!vm_execute("update")) {
                    context()->signals.push({SIGNAL_EXCEPTION, vm_error()});
                    return;
                };
            }
        }

        timer_consume(steps);
    }

    void executor_draw() {
        if (!vm_execute("draw")) {
            context()->signals.push({SIGNAL_EXCEPTION, vm_error()});
            return;
        };

        if (state.paused) {
            static const std::string menu[] = {"RESUME", "EXIT"};

            for (auto i = 0; i < 2; i++) {
                auto x = 1;
                auto y = 110 + 9 * i;

                for (const auto &ch : menu[i]) {
                    if (state.select == i) {
                        painter_char(ch, x, y, 3);
                    } else {
                        painter_char(ch, x, y, 1);
                    }
                    x += 4;
                }
            }
        }
    }

    void executor_enter() {
        if (!vm_initialize(context()->script)) {
            ctx_signals().push({SIGNAL_EXCEPTION, vm_error()});
            return;
        }

        if (!vm_execute("init")) {
            ctx_signals().push({SIGNAL_EXCEPTION, vm_error()});
            return;
        };

        painter_clear(0);

        context()->signals.push({SIGNAL_STOP_INPUT});

        timer_reset();
    }

    void executor_leave() {
        vm_release();
    }

}
