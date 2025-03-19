
#include <iostream>

#include "t8_emulator.h"

int main(int argc, char *argv[]) {
    if (t8::emulator_initialize()) {
        t8::emulator_run();
        t8::emulator_quit();
    }
    
    return 0;
}
