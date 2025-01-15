#include "emulator.h"
#include <iostream>

int main(int argc, char *argv[]) {

    t8y::Emulator e;

    if (e.initialize()) {
        e.run();
        e.quit();
    }

    return 0;
}
