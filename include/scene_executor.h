#pragma once

#include <string>
#include <vector>

#include <sol2/include/sol/sol.hpp>
#include <tuple>

namespace t8::core
{
    struct AppContext;
}

namespace t8::scene::executor
{
    using namespace t8::core;

    struct ExecutorVM
    {
        sol::state lua;
        sol::protected_function init;
        sol::protected_function update;
        sol::protected_function draw;
    };

    struct ExecutorState
    {
        uint8_t select;
        bool paused{false};
        std::optional<ExecutorVM> vm;
    };

    void update(AppContext *ctx, ExecutorState *state);

    void draw(AppContext *ctx, ExecutorState *state);

    void enter(AppContext *ctx, ExecutorState *state);

    void leave(AppContext *ctx, ExecutorState *state);
}