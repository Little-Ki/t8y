#pragma once

#include <string>
#include <vector>

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::console
{
    using namespace t8::core;

    struct ConsoleRecord
    {
        int front_height;
        int height;
        bool prefix;
        uint8_t color;
        std::string text;
    };

    struct ConsoleHistory
    {
        std::vector<std::string> texts;
        size_t index;
        bool use = false;
    };

    struct ConsoleState
    {
        std::vector<std::string> history;
        size_t history_index = 0;
        bool use_history = false;

        std::vector<ConsoleRecord> records;
        std::string input;
        size_t cursor = 0;
    };

    void update(AppContext *ctx, ConsoleState *state);

    void draw(AppContext *ctx, ConsoleState *state);

    void enter(AppContext *ctx, ConsoleState *state);

    void leave(AppContext *ctx, ConsoleState *state);

    void print(ConsoleState *state, const std::string &text, bool prefix = false, uint8_t color = 1);
}