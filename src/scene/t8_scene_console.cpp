#include "t8_scene_console.h"
#include "t8_core_context.h"
#include "t8_core_painter.h"
#include "t8_input_keyboard.h"
#include "t8_utils_algo.h"
#include "t8_utils_storage.h"

#include "t8_constants.h"

#include <algorithm>
#include <cmath>

using namespace t8::utils;
using namespace t8::input;
using namespace t8::core;

namespace t8::scene {

    struct ConsoleRecord {
        int front_height;
        int height;
        bool prefix;
        uint8_t color;
        std::string text;
    };

    struct ConsoleHistory {
        std::vector<std::string> texts;
        size_t index;
        bool use = false;
    };

    struct ConsoleState {
        std::vector<std::string> history;
        size_t history_index = 0;
        bool use_history = false;

        std::vector<ConsoleRecord> records;
        std::string input;
        size_t cursor = 0;

        bool first_time = true;
    };

    static ConsoleState state;

    void console_sanitize_cursor() {
        state.cursor = std::clamp(state.cursor, 0ULL, state.input.size());
    }

    int console_measure_height(const size_t indent, const std::string &text) {
        int height = 8;
        size_t x = indent;

        for (auto &c : text) {
            if (c == '\r')
                continue;
            x += 4;
            if (x > 120 || c == '\n') {
                height += 8;
                x = indent;
            }
        }

        return height;
    }

    void console_print(const std::string &text, bool prefix = false, uint8_t color = 1) {
        if (state.records.empty()) {
            state.records.push_back({0,
                                     console_measure_height(prefix ? 8 : 0, text),
                                     prefix, color, text});
        } else {
            const auto &record = state.records.back();
            int front_height = record.front_height + record.height;

            state.records.push_back({front_height,
                                     console_measure_height(prefix ? 8 : 0, text),
                                     prefix, color, text});
        }
    }

    void console_reset() {
        state.records.clear();

        console_print("T8Y FANTASTIC CONSOLE", false, 3);
        console_print("");
        console_print("Type help for help");
    }

    bool console_validate(const std::vector<std::string> payload, uint32_t count) {
        if (payload.size() < count) {
            console_print("Too few argument", false, 3);
            return false;
        }
        if (payload.size() > count) {
            console_print("Too many argument", false, 3);
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
            console_print("Commands:", false, 6);
            console_print("");
            console_print("load <filename>");
            console_print("run");
            console_print("save <filename>");
            console_print("cls");
            console_print("");
            console_print("Press esc to editor view");
            return true;
        }

        if (str_equals(payload[0], "cls")) {
            console_reset();
            return true;
        }

        if (str_equals(payload[0], "load") && payload.size() > 1) {
            if (!storage_load_cart(payload[1]))
                console_print("Failed to load cart", false, 3);
            return true;
        }

        if (str_equals(payload[0], "save") && payload.size() > 1) {
            if (!storage_save_cart(payload[1]))
                console_print("Failed to save cart", false, 3);
            return true;
        }

        if (str_equals(payload[0], "run")) {
            if (console_validate(payload, 1)) {
                ctx_signals().push({SIGNAL_SWAP_EXECUTOR});
                return true;
            }
        }

        console_print("Invalid syntax", false, 3);

        return true;
    }

    void console_update() {
        if (keyboard_pressed(SCANCODE_ESC)) {
            ctx_signals().push({SIGNAL_SWAP_EDITOR});
            return;
        }

        if (!ctx_inputs().empty()) {
            const auto text = ctx_inputs().back();
            ctx_inputs().pop();

            for (const auto &ch : text)
                if (!(ch & 0x80) && state.input.size() < 64) {
                    state.input.insert(std::next(state.input.begin(), state.cursor), ch);
                    state.cursor++;
                }
            console_sanitize_cursor();
            state.use_history = false;
            return;
        }

        if (keyboard_pressed(SCANCODE_RETURN) ||
            keyboard_pressed(SCANCODE_ENTER)) {
            console_print(state.input, true, 1);
            if (console_command())
                if (state.history.empty() || state.history.back() != state.input) {
                    state.history.push_back(state.input);
                }
            state.use_history = false;
            state.input.clear();
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_BACKSPACE)) {
            if (!state.input.empty() && state.cursor > 0) {
                state.input.erase(std::next(state.input.begin(), state.cursor - 1));
                state.cursor--;
            }
            return;
        }
        if (keyboard_triggered(SCANCODE_DELETE)) {
            if (!state.input.empty() && state.cursor < state.input.size()) {
                state.input.erase(std::next(state.input.begin(), state.cursor));
            }
            return;
        }

        if (keyboard_triggered(SCANCODE_LEFT)) {
            if (state.cursor > 0)
                state.cursor -= 1;
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_RIGHT)) {
            state.cursor += 1;
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_UP)) {
            auto has_history = false;
            if (!state.use_history && state.history.size() > 0) {
                state.history_index = state.history.size() - 1;
                state.use_history = true;
                has_history = true;
            } else if (state.history_index > 0) {
                state.history_index = std::clamp(state.history_index - 1, 0ULL, state.history.size() - 1);
                has_history = true;
            }

            if (has_history) {
                state.input = state.history[state.history_index];
                state.cursor = state.input.size();
            }
            return;
        }

        if (keyboard_triggered(SCANCODE_DOWN)) {
            if (state.use_history) {
                state.history_index = std::clamp(state.history_index + 1, 0ULL, state.history.size());
                if (state.history_index == state.history.size()) {
                    state.input.clear();
                } else {
                    state.input = state.history[state.history_index];
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
        painter_clear(0);

        auto front_height = 0;
        auto input_height = console_measure_height(8, state.input);

        if (state.records.size() > 0) {
            const auto record = state.records.back();
            front_height = record.front_height + record.height;
        }

        auto start = std::lower_bound(
            state.records.begin(),
            state.records.end(),
            std::max(0, front_height - 128),
            [](const ConsoleRecord &r, int value) {
                return r.front_height < value;
            });

        auto x = 0, y = std::min(0, 128 - front_height - input_height);

        if (start != state.records.end()) {
            y += start->front_height;
        }

        for (auto it = start; it != state.records.end(); it += 1) {
            if (it->prefix) {
                painter_char('>', 0, y, it->color);
                x = 8;
            }

            for (const auto &ch : it->text) {
                painter_char(ch, x, y, it->color);
                x += 4;

                if (x > 120) {
                    x = it->prefix ? 8 : 0;
                    y += 8;
                }
            }

            x = 0;
            y += 8;
        }

        painter_char('>', 0, y);
        x = 8;

        for (auto i = 0ULL; i <= state.input.size(); i++) {
            if (state.cursor == i) {
                if ((timer_ticks() >> 5) % 2) {
                    painter_rect(x, y, 1, 8, 3);
                }
            }
            if (i < state.input.size()) {
                painter_char(state.input[i], x, y);
                x += 4;
                if (x > 120) {
                    x = 8;
                    y += 8;
                }
            }
        }
    }

    void console_enter() {
        ctx_signals().push({SIGNAL_START_INPUT});
        painter_reset();

        if (state.first_time) {
            state.first_time = false;
            console_reset();
        }

        timer_reset();
    }

    void console_leave() {
        ctx_signals().push({SIGNAL_STOP_INPUT});
    }

    void console_print(const std::string &text, bool err) {
        if (err) {
            console_print(text, false, 3);
        } else {
            console_print(text);
        }
    }
}