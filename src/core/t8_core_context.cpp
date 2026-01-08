#include "t8_core_context.h"
#include <cassert>

namespace t8::core {
    AppContext *context() {
        static auto instance = new AppContext();
        return instance;
    }

    bool signal_empty() {
        return context()->signals.empty();
    }

    void signal_push(uint32_t type, signal_value_t value) {
        return context()->signals.push({type, value});
    }

    SignalRecord signal_pop() {
        assert(!signal_empty());
        const auto record = std::move(context()->signals.front());
        context()->signals.pop();
        return record;
    }

    bool input_empty() {
        return false;
    }

    void input_push(std::string &&text) {
        context()->inputs.push(std::move(text));
    }

    std::string input_pop() {
        assert(!input_empty());
        const auto record = std::move(context()->inputs.front());
        context()->inputs.pop();
        return record;
    }

    void set_script(std::string &&text) {
        context()->script = std::move(text);
    }

    const std::string &get_script() {
        return context()->script;
    }
}