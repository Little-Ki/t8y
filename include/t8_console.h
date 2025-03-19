#pragma once

#include <string>
#include <vector>

#include "t8_timer.h"

namespace t8 {

    struct ConsoleLine {
        bool prefix;
        uint8_t color;
        std::string content;
    };

    using ConsoleLines = std::vector<ConsoleLine>;

    struct ConsoleState {
        struct {
            std::vector<std::string> items;
            size_t index;
            bool use{false};
        } history;
        bool first_time{true};
        std::string input;
        size_t cursor{0};
        ConsoleLines lines;
        Timer timer; 
    };

    void console_update();

    void console_draw();

    void console_enter();

    void console_leave();

    void console_print(const std::string &text, bool err);

}