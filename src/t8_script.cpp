#include "t8_script.h"

namespace t8 {

    std::string _script;

    void t8::script_set(std::string &text) {
        _script = text;
    }
    
    std::string &t8::script_get() {
        return _script;
    }
    
}