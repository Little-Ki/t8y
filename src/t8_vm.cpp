#include "t8_vm.h"

namespace t8 {

    VMState state;
    
    int vm_call_clear() {
        return 0;
    }

    bool vm_initialize(const std::string &src) {
        return true;
    }

    void vm_clear() {
    }

    const std::string &vm_error() {
        return state.err_msg;
    }

    bool vm_execute(const char *fn_name) {
        return true;
    }

}