#include "t8_core_context.h"
#include <cassert>

namespace t8::core {
    AppContext *context() {
        static auto instance = new AppContext();
        return instance;
    }

    std::queue<std::string> &ctx_inputs()
    {
        return context()->inputs;
    }

    std::queue<SignalRecord> &ctx_signals()
    {
        return context()->signals;
    }

    std::string &ctx_script()
    {
        return context()->script;
    }
}