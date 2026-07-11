#pragma once
#include <string>

namespace t8::scene {
    void update_script_editor();

    void draw_script_editor();

    void script_editor_set_script(const std::string &script);

    std::string script_editor_get_script();
}