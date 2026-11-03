
#include <iostream>

#include "core_emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    t8::core::Emulator emulator;

    AppContext ctx;

    if (emulator.init())
    {
        emulator.run();
        emulator.quit();
    }

    return 0;
}
