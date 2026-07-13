
#include <iostream>
#include <memory>

#include "core/emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    auto ctx = std::make_unique<AppContext>();

    if (emu_init(ctx.get()))
    {
        emu_run(ctx.get());
        emu_quit(ctx.get());
    }

    return 0;
}
