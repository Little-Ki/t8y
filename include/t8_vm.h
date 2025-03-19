#pragma once

#include <string>

#include "t8_timer.h"

namespace t8 {

    struct VMState {
        std::string err_msg;
    };

    bool vm_initialize(const std::string &src);

    void vm_clear();

    const std::string &vm_error();

    bool vm_execute(const char *fn_name);
}