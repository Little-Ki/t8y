#pragma once
#include <functional>
#include <unordered_map>

#include "core/context.h"

namespace t8::core
{
    bool emu_init(AppContext *ctx);

    void emu_run(AppContext *ctx);

    void emu_quit(AppContext *ctx);
}