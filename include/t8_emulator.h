#pragma once

namespace t8 {

    enum class Scene {
        Editor,
        Console,
        Executor
    };

    struct EnumlatorState {
        Scene scene;
        float pixel_size;
    };

    bool emulator_initialize();
    
    void emulator_run();

    void emulator_quit();

}