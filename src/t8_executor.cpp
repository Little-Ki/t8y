#include "t8_executor.h"
#include "t8_graphic.h"
#include "t8_keybd.h"
#include "t8_script.h"
#include "t8_signal.h"
#include "t8_vm.h"

#include <algorithm>

namespace t8 {

    ExecutorState state;

    void executor_update() {
        if (keybd_pressed(41) && !state.paused) {
            state.paused = true;
            state.select = 0;
        }

        if (state.paused) {
            if (keybd_pressed(82) && state.select) {
                state.select -= 1;
            }
            if (keybd_pressed(81)) {
                state.select = std::clamp(state.select + 1, 0, 1);
            }
            if (keybd_pressed(40) || keybd_pressed(88)) {
                state.paused = false;
                if (state.select == 1) {
                    signal_send(Signal::SwapConsole);
                }
            }
        }

        auto steps = state.timer.steps();
        if (steps < 3 && !state.paused) {
            for (auto i = 0; i < steps; i++) {
                if (!vm_execute("update")) {
                    signal_send(Signal::Exception, vm_error());
                    return;
                };
            }
        }
        state.timer.consume(steps);
    }

    void executor_draw() {
        if (!vm_execute("draw")) {
            signal_send(Signal::Exception, vm_error());
            return;
        };

        if (state.paused) {
            static const std::string menu[] = {"RESUME", "EXIT"};

            graphic_backup_palette();
            graphic_reset_palette();

            for (auto i = 0; i < 2; i++) {
                auto x = 1;
                auto y = 110 + 9 * i;

                for (const auto &ch : menu[i]) {
                    if (state.select == i) {
                        graphic_char(ch, x, y, 3);
                    } else {
                        graphic_char(ch, x, y, 1);
                    }
                    x += 4;
                }
            }

            graphic_restore_palette();
        }
    }

    void executor_enter() {

        state.timer.reset();

        if (!vm_initialize(script_get())) {
            signal_send(Signal::Exception, vm_error());
            return;
        }

        if (!vm_execute("init")) {
            signal_send(Signal::Exception, vm_error());
            return;
        };

        graphic_clear(0);

        signal_send(Signal::StopInput);
    }

    void executor_leave() {
        vm_release();
    }

}
