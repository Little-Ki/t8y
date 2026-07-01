
#include <iostream>

#include "core/emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    AppContext ctx;

    if (emulator_init(ctx))
    {
        emulator_run(ctx);
        emulator_quit(ctx);
    }

    return 0;
}
