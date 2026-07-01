#pragma once
#include <functional>
#include <unordered_map>

#include "core/context.h"

namespace t8::core
{
    bool emulator_init(AppContext &ctx);

    void emulator_run(AppContext &ctx);

    void emulator_quit(AppContext &ctx);
}