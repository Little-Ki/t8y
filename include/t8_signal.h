#pragma once
#include <any>
#include <utility>

namespace t8 {
    enum class Signal {
        SwapConsole,
        SwapEditor,
        SwapExecutor,
        StartInput,
        StopInput,
        Exception,
        Print
    };

    using signal_t = std::pair<Signal, std::any>;

    bool signal_empty();

    void signal_push(Signal type, std::any value = {});

    const signal_t& signal_peek();

    void signal_pop();

}