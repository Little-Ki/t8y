
#include <iostream>
#include <memory>

#include "core_emulator.h"

using namespace t8::core;

int main(int argc, char *argv[])
{
    t8::core::Emulator emulator;

    if (emulator.init())
    {
        emulator.run();
        emulator.quit();
    }

    return 0;
}
