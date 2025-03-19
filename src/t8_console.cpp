#include "t8_console.h"
#include "t8_graphic.h"
#include "t8_keybd.h"
#include "t8_signal.h"
#include "t8_sinput.h"
#include "t8_utils.h"

#include <algorithm>
#include <cmath>

namespace t8 {
    ConsoleState state{};

    void console_sanitize_cursor() {
        state.cursor = std::clamp(state.cursor, 0ULL, state.input.size());
    }

    int console_measure_height(const ConsoleLine &line) {
        const auto line_width = line.prefix ? 29 : 31;

        if (line.content.empty())
            return 8;

        return static_cast<int>(std::ceil(line.content.size() / static_cast<float>(line_width))) << 3;
    }

    int console_measure_lines() {
        auto height = 0;
        for (auto &line : state.lines) {
            height += console_measure_height(line);
        }

        return height;
    }

    int console_measure_input() {
        return static_cast<int>(std::ceil(state.input.size() / 29)) << 3;
    }

    void console_join(const std::string &text, bool prefix = false, uint8_t color = 1) {
        state.lines.push_back({prefix,
                               color,
                               ""});
        for (auto it = text.begin(); it != text.end(); it++) {
            const auto ch = *it;

            if (ch == '\n') {
                state.lines.push_back({prefix, color, ""});
            } else if (ch != '\r') {
                state.lines.back().content.push_back(ch);
            }
        }
    }

    void console_clear() {
        state.lines.clear();

        console_join("T8Y FANTASTIC CONSOLE", false, 3);
        console_join("");
        console_join("Type help for help");
    }

    bool console_validate(const std::vector<std::string> payload, uint32_t count) {
        if (payload.size() < count) {
            console_join("Too few argument", false, 3);
            return false;
        }
        if (payload.size() > count) {
            console_join("Too many argument", false, 3);
            return false;
        }

        return true;
    }

    bool console_command() {
        if (state.input.empty())
            return false;

        auto payload = str_explode(state.input, ' ');

        for (auto &str : payload) {
            str_ltrim(str);
            str_rtrim(str);
        }

        if (payload[0].empty())
            return false;

        if (str_equals(payload[0], "help")) {
            console_join("Commands:", false, 6);
            console_join("");
            console_join("load <filename>");
            console_join("run");
            console_join("save <filename>");
            console_join("cls");
            console_join("");
            console_join("Press esc to editor view");
            return true;
        }

        if (str_equals(payload[0], "cls")) {
            console_clear();
            return true;
        }

        if (str_equals(payload[0], "load")) {
            return true;
        }

        if (str_equals(payload[0], "save") && payload.size() > 1) {
            return true;
        }

        if (str_equals(payload[0], "run")) {
            if (console_validate(payload, 1)) {
                signal_send(Signal::SwapExecutor);
                return true;
            }
        }

        console_join("Invalid syntax", false, 3);

        return true;
    }

    void console_update() {

        if (keybd_pressed(41)) {
            signal_send(Signal::SwapEditor);
            return;
        }

        if (!sinput_empty()) {
            const auto text = sinput_peek();
            sinput_pop();

            for (const auto &ch : text)
                if (!(ch & 0x80)) {
                    state.input.insert(std::next(state.input.begin(), state.cursor), ch);
                    state.cursor++;
                }
            console_sanitize_cursor();
            state.history.use = false;
            return;
        }

        if (keybd_pressed(40) || keybd_pressed(88)) {
            console_join(state.input, true, 1);
            if (console_command())
                if (state.history.items.empty() || state.history.items.back() != state.input) {
                    state.history.items.push_back(state.input);
                }
            state.history.use = false;
            state.input.clear();
            console_sanitize_cursor();
            return;
        }

        if (keybd_triggered(42)) {
            if (!state.input.empty() && state.cursor > 0) {
                state.input.erase(std::next(state.input.begin(), state.cursor - 1));
                state.cursor--;
            }
            return;
        }

        if (keybd_triggered(80)) {
            if (state.cursor)
                state.cursor--;
            console_sanitize_cursor();
            return;
        }

        if (keybd_triggered(79)) {
            state.cursor++;
            console_sanitize_cursor();
            return;
        }

        if (keybd_triggered(82)) {
            if (!state.history.use) {
                state.history.index = state.history.items.size() - 1;
                state.history.use = true;
            } else if (state.history.index > 0) {
                state.history.index = std::clamp(state.history.index - 1, 0ULL, state.history.items.size() - 1);
            }

            state.input = state.history.items[state.history.index];
            state.cursor = state.input.size();
            return;
        }

        if (keybd_triggered(81)) {
            if (state.history.use) {
                state.history.index = std::clamp(state.history.index + 1, 0ULL, state.history.items.size());
                if (state.history.index == state.history.items.size()) {
                    state.input.clear();
                } else {
                    state.input = state.history.items[state.history.index];
                    state.cursor = state.input.size();
                }
            } else {
                state.input.clear();
            }

            console_sanitize_cursor();
            return;
        }
    }

    void console_draw() {
        graphic_clear(0);

        auto linesHeight = console_measure_lines();
        auto inputHeight = console_measure_input();
        auto x = 0, y = std::min(0, 112 - linesHeight);

        if (inputHeight > 8) {
            y -= (inputHeight - 8);
        }

        for (const auto &line : state.lines) {
            if (line.prefix) {
                graphic_char('>', x, y, line.color);
                x = 8;
            }

            for (const auto &ch : line.content) {
                graphic_char(ch, x, y, line.color);
                x += 4;

                if (x > 120) {
                    x = line.prefix ? 8 : 0;
                    y += 8;
                }
            }

            x = 0;
            y += 8;
        }

        graphic_char('>', 0, y);
        x = 8;

        for (auto i = 0ULL; i <= state.input.size(); i++) {
            if (state.cursor == i) {
                if ((state.timer.ticks() >> 5) % 2) {
                    graphic_rect(x, y, 1, 7, 3);
                }
            }
            if (i < state.input.size()) {
                graphic_char(state.input[i], x, y);
                x += 4;
                if (x > 120) {
                    x = 8;
                    y += 8;
                }
            }
        }
    }

    void console_enter() {
        signal_send(Signal::StartInput);
        graphic_reset();

        if (state.first_time) {
            state.first_time = false;
            console_clear();
        }

        state.timer.reset();
    }

    void console_leave() {
        signal_send(Signal::StopInput);
    }

    void console_print(const std::string &text, bool err) {
        if (err) {
            console_join(text, false, 3);
        } else {
            console_join(text);
        }
    }
}