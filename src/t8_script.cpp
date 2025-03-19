#include "t8_script.h"

namespace t8 {

    std::string _script;

    void script_set(std::string &text) {
        _script = text;
    }
    
    std::string &script_get() {
        return _script;
    }
    
}