
#include <iostream>
#include <memory>

#include "core/emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    auto ctx = std::make_unique<AppContext>();

    if (emu_init(*ctx))
    {
        emu_run(*ctx);
        emu_quit(*ctx);
    }

    return 0;
}
