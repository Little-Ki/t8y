#pragma once

#include <string>

namespace t8 {

    bool sinput_empty();

    void sinput_push(const std::string& text);

    void sinput_pop();

    const std::string& sinput_peek();

}