#pragma once
#include <queue>
#include <string>
#include <variant>

namespace t8::core {

    using signal_value_t = std::variant<std::monostate, std::string>;

    struct SignalRecord {
        uint32_t type;
        signal_value_t value;
    };

    struct AppContext {
        std::queue<std::string> inputs;
        std::queue<SignalRecord> signals;
        std::string script;
    };

    AppContext *context();

    bool signal_empty();
    void signal_push(uint32_t type, signal_value_t value = std::monostate());
    SignalRecord signal_pop();

    bool input_empty();
    void input_push(std::string &&text);
    std::string input_pop();

    void set_script(std::string&& text);
    const std::string& get_script();

}