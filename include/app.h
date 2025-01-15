#pragma once

#include "app_context.h"
#include "graphic.h"
#include "input.h"
#include "signals.h"

namespace t8y {

    class App {
    public:
        AppContext context;
        MouseInput mouse;
        KayboardInput keyboard;
        GamepadInput gamepad;
        TextInput texts;
        Graphic graphic;
        SignalQueue signal;

        App();
    };

}