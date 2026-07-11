#pragma once
#include <functional>
#include <unordered_map>

#include "core/context.h"

namespace t8::core::emulator
{
    bool init(AppContext &ctx);

    void run(AppContext &ctx);

    void quit(AppContext &ctx);
}