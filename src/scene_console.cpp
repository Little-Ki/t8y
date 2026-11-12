#include "scene_console.h"
#include "cartridge.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_keyboard.h"

#include "utils_algo.h"

#include "constants.h"

#include <algorithm>

using namespace t8::input;
using namespace t8::utils;

namespace t8::scene::console {

    inline void sanitize_cursor(ConsoleState *state) {
        state->cursor = std::clamp(state->cursor, 0ULL, state->input.size());
    }

    static int measure_height(const size_t indent, const std::string &text) {
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

    static bool validate(ConsoleState *state, const std::vector<std::string> payload, uint32_t count) {
        if (payload.size() < count) {
            print(state, "Too few argument", false, 3);
            return false;
        }
        if (payload.size() > count) {
            print(state, "Too many argument", false, 3);
            return false;
        }

        return true;
    }

    static void reset(ConsoleState *state) {
        state->records.clear();

        print(state, "T8Y FANTASTIC CONSOLE", false, 3);
        print(state, "");
        print(state, "Type help for help");
    }

    static bool command(AppContext *ctx, ConsoleState *state) {
        if (state->input.empty())
            return false;

        auto payload = str_explode(state->input, ' ');

        for (auto &str : payload) {
            str_ltrim(str);
            str_rtrim(str);
        }

        if (payload[0].empty())
            return false;

        if (str_equals(payload[0], "help")) {
            print(state, "Commands:", false, 6);
            print(state, "");
            print(state, "load <filename> - load cart");
            print(state, "save <filename> - save cart");
            print(state, "run - strat script");
            print(state, "cls - clear console");
            print(state, "");
            print(state, "Press esc to editor view");
            return true;
        }

        if (str_equals(payload[0], "cls")) {
            reset(state);
            return true;
        }

        if (str_equals(payload[0], "load") && payload.size() > 1) {
            if (cartridge::load(payload[1], ctx))
                print(state, "Cartridge loaded", false, 6);
            else
                print(state, "Failed to load cartridge", false, 3);
            return true;
        }

        if (str_equals(payload[0], "save") && payload.size() > 1) {
            if (cartridge::save(payload[1], ctx))
                print(state, "Cartridge saved", false, 6);
            else
                print(state, "Failed to save cartridge", false, 3);
            return true;
        }

        if (str_equals(payload[0], "run")) {
            if (validate(state, payload, 1)) {
                ctx->signals.push({SIGNAL_SWAP_EXECUTOR});
                return true;
            }
        }

        print(state, "Invalid syntax", false, 3);

        return true;
    }

    void update(AppContext *ctx, ConsoleState *state) {
        if (keyboard::pressed(ctx->keyboard, SCANCODE_ESC)) {
            ctx->signals.push({SIGNAL_SWAP_EDITOR});
            return;
        }

        if (!ctx->inputs.empty()) {
            const auto text = ctx->inputs.front();
            ctx->inputs.pop();

            for (const auto &ch : text)
                if (!(ch & 0x80) && state->input.size() < 64) {
                    state->input.insert(std::next(state->input.begin(), state->cursor), ch);
                    state->cursor++;
                }
            state->use_history = false;
            sanitize_cursor(state);
            return;
        }

        if (keyboard::pressed(ctx->keyboard, SCANCODE_RETURN) ||
            keyboard::pressed(ctx->keyboard, SCANCODE_ENTER)) {
            print(state, state->input, true, 1);
            if (command(ctx, state))
                if (state->history.empty() || state->history.back() != state->input) {
                    state->history.push_back(state->input);
                }
            state->use_history = false;
            state->input.clear();
            sanitize_cursor(state);
            return;
        }

        if (keyboard::triggered(ctx->keyboard, SCANCODE_BACKSPACE)) {
            if (!state->input.empty() && state->cursor > 0) {
                state->input.erase(std::next(state->input.begin(), state->cursor - 1));
                state->cursor--;
            }
            return;
        }
        if (keyboard::triggered(ctx->keyboard, SCANCODE_DELETE)) {
            if (!state->input.empty() && state->cursor < state->input.size()) {
                state->input.erase(std::next(state->input.begin(), state->cursor));
            }
            return;
        }

        if (keyboard::triggered(ctx->keyboard, SCANCODE_LEFT)) {
            if (state->cursor > 0)
                state->cursor -= 1;
            sanitize_cursor(state);
            return;
        }

        if (keyboard::triggered(ctx->keyboard, SCANCODE_RIGHT)) {
            state->cursor += 1;
            sanitize_cursor(state);
            return;
        }

        if (keyboard::triggered(ctx->keyboard, SCANCODE_UP)) {
            auto has_history = false;
            if (!state->use_history && state->history.size() > 0) {
                state->history_index = state->history.size() - 1;
                state->use_history = true;
                has_history = true;
            } else if (state->history_index > 0) {
                state->history_index = std::clamp(state->history_index - 1, 0ULL, state->history.size() - 1);
                has_history = true;
            }

            if (has_history) {
                state->input = state->history[state->history_index];
                state->cursor = state->input.size();
            }
            return;
        }

        if (keyboard::triggered(ctx->keyboard, SCANCODE_DOWN)) {
            if (state->use_history) {
                state->history_index = std::clamp(state->history_index + 1, 0ULL, state->history.size());
                if (state->history_index == state->history.size()) {
                    state->input.clear();
                } else {
                    state->input = state->history[state->history_index];
                    state->cursor = state->input.size();
                }
            } else {
                state->input.clear();
            }

            sanitize_cursor(state);
            return;
        }
    }

    void draw(AppContext *ctx, ConsoleState *state) {
        gfx::clear(ctx->memory, 0);

        auto &records = state->records;

        auto front_height = 0;
        auto input_height = measure_height(8, state->input);

        if (state->records.size() > 0) {
            const auto record = state->records.back();
            front_height = record.front_height + record.height;
        }

        auto x = 0, y = std::min(0, 128 - front_height - input_height);

        auto start = std::upper_bound(
            state->records.begin(),
            state->records.end(),
            std::max(0, front_height - 128),
            [](int value, const ConsoleRecord &r) {
                return value < r.front_height;
            });

        if (start != state->records.end()) {
            start -= 1;
        }

        if (start != state->records.end()) {
            y += start->front_height;
        }

        for (auto it = start; it != state->records.end(); it += 1) {
            if (it->prefix) {
                gfx::draw_char(ctx->memory, '>', 0, y, it->color);
                x = 8;
            }

            for (const auto &ch : it->text) {
                gfx::draw_char(ctx->memory, ch, x, y, it->color);
                x += 4;

                if (x > 120) {
                    x = it->prefix ? 8 : 0;
                    y += 8;
                }
            }

            x = 0;
            y += 8;
        }

        gfx::draw_char(ctx->memory, '>', 0, y);
        x = 8;

        for (auto i = 0ULL; i <= state->input.size(); i++) {
            if (state->cursor == i) {
                if ((ctx->timer.ticks() >> 5) % 2) {
                    gfx::draw_rect(ctx->memory, x, y, 1, 8, 3);
                }
            }
            if (i < state->input.size()) {
                gfx::draw_char(ctx->memory, state->input[i], x, y);
                x += 4;
                if (x > 120) {
                    x = 8;
                    y += 8;
                }
            }
        }
    }

    void enter(AppContext *ctx, ConsoleState *state) {
        ctx->signals.push({SIGNAL_START_INPUT});
        gfx::reset_all(ctx->memory);

        if (state->records.empty()) {
            reset(state);
        }
    }

    void leave(AppContext *ctx, ConsoleState *state) {
        ctx->signals.push({SIGNAL_STOP_INPUT});
    }

    void print(ConsoleState *state, const std::string &text, bool prefix, uint8_t color) {
        if (state->records.empty()) {
            state->records.push_back({0,
                                      measure_height(prefix ? 8 : 0, text),
                                      prefix, color, text});
        } else {
            const auto &record = state->records.back();
            int front_height = record.front_height + record.height;

            state->records.push_back({front_height,
                                      measure_height(prefix ? 8 : 0, text),
                                      prefix, color, text});
        }
    }
}