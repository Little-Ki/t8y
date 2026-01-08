
#include <iostream>

#include "t8_core_emulator.h"

using namespace t8::core;

int main(int argc, char *argv[]) {
    if (emulator_initialize()) {
        emulator_run();
        emulator_quit();
    }
    
    return 0;
}
