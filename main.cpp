
#include <iostream>

#include "core/emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    AppContext ctx;

    if (emulator::init(ctx))
    {
        emulator::run(ctx);
        emulator::quit(ctx);
    }

    return 0;
}
