
#include <iostream>
#include <memory>

#include "core_emulator.h"
#include "core_window.h"

using namespace t8::core;

int main(int argc, char* argv[])
{
	Emulator emulator;

	if (emulator.init())
	{
		emulator.run();
		emulator.quit();
	}

	return 0;
}
