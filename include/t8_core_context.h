#pragma once
#include <queue>
#include <string>
#include <variant>

namespace t8::core
{

    using signal_value_t = std::variant<std::monostate, std::string>;

    struct SignalRecord
    {
        uint32_t type;
        signal_value_t value;
    };

    struct AppContext
    {
        std::queue<std::string> inputs;
        std::queue<SignalRecord> signals;
        std::string script;
    };

    AppContext *context();

    std::queue<std::string> &ctx_inputs();
    std::queue<SignalRecord> &ctx_signals();
    std::string& ctx_script();
}