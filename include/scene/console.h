#pragma once

#include <string>
#include <vector>

namespace t8::scene
{
    void console_update();

    void console_draw();

    void console_enter();

    void console_leave();

    void console_print(const std::string &text, bool err);
}