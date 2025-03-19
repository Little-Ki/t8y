#include "t8_script.h"

namespace t8 {

    std::string _script;

    void script_set(const std::string &text) {
        _script = text;
    }
    
    const std::string &script_get() {
        return _script;
    }
    
}