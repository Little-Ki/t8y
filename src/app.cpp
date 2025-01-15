#include "app.h"

namespace t8y {
    App::App() : context({}), graphic(&context), mouse(&context),
                 keyboard(&context), gamepad(&context) {
    }
}