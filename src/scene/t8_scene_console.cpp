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

    bool first_time = true;
    ConsoleHistory history;
    std::vector<ConsoleRecord> records;
    std::string input;
    size_t cursor = 0;

    void console_sanitize_cursor() {
        cursor = std::clamp(cursor, 0ULL, input.size());
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
        if (records.empty()) {
            records.push_back({0,
                               console_measure_height(prefix ? 8 : 0, text),
                               prefix, color, text});
        } else {
            const auto &record = records.back();
            int front_height = record.front_height + record.height;

            records.push_back({front_height,
                               console_measure_height(prefix ? 8 : 0, text),
                               prefix, color, text});
        }
    }

    void console_reset() {
        records.clear();

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
        if (input.empty())
            return false;

        auto payload = str_explode(input, ' ');

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
                if (!(ch & 0x80) && input.size() < 64) {
                    input.insert(std::next(input.begin(), cursor), ch);
                    cursor++;
                }
            console_sanitize_cursor();
            history.use = false;
            return;
        }

        if (keyboard_pressed(SCANCODE_RETURN) ||
            keyboard_pressed(SCANCODE_ENTER)) {
            console_print(input, true, 1);
            if (console_command())
                if (history.texts.empty() || history.texts.back() != input) {
                    history.texts.push_back(input);
                }
            history.use = false;
            input.clear();
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_BACKSPACE)) {
            if (!input.empty() && cursor > 0) {
                input.erase(std::next(input.begin(), cursor - 1));
                cursor--;
            }
            return;
        }
        if (keyboard_triggered(SCANCODE_DELETE)) {
            if (!input.empty() && cursor < input.size()) {
                input.erase(std::next(input.begin(), cursor));
            }
            return;
        }

        if (keyboard_triggered(SCANCODE_LEFT)) {
            if (cursor > 0)
                cursor -= 1;
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_RIGHT)) {
            cursor += 1;
            console_sanitize_cursor();
            return;
        }

        if (keyboard_triggered(SCANCODE_UP)) {
            if (!history.use) {
                history.index = history.texts.size() - 1;
                history.use = true;
            } else if (history.index > 0) {
                history.index = std::clamp(history.index - 1, 0ULL, history.texts.size() - 1);
            }

            input = history.texts[history.index];
            cursor = input.size();
            return;
        }

        if (keyboard_triggered(SCANCODE_DOWN)) {
            if (history.use) {
                history.index = std::clamp(history.index + 1, 0ULL, history.texts.size());
                if (history.index == history.texts.size()) {
                    input.clear();
                } else {
                    input = history.texts[history.index];
                    cursor = input.size();
                }
            } else {
                input.clear();
            }

            console_sanitize_cursor();
            return;
        }
    }

    void console_draw() {
        painter_clear(0);

        auto frontHeight = 0;
        auto inputHeight = console_measure_height(8, input);

        if (records.size() > 0) {
            const auto record = records.back();
            frontHeight = record.front_height + record.height;
        }

        auto start = std::lower_bound(
            records.begin(),
            records.end(),
            std::max(0, frontHeight - 128),
            [](const ConsoleRecord &r, int value) {
                return r.front_height < value;
            });

        auto x = 0, y = std::min(0, 128 - frontHeight - inputHeight);

        if (start != records.end()) {
            y += start->front_height;
        }

        for (auto it = start; it != records.end(); it += 1) {
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

        for (auto i = 0ULL; i <= input.size(); i++) {
            if (cursor == i) {
                if ((timer_ticks() >> 5) % 2) {
                    painter_rect(x, y, 1, 7, 3);
                }
            }
            if (i < input.size()) {
                painter_char(input[i], x, y);
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

        if (first_time) {
            first_time = false;
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