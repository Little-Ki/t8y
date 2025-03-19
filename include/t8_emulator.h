#pragma once
#include <functional>
#include <unordered_map>


namespace t8 {

    enum class Scene {
        Editor,
        Console,
        Executor
    };

    struct SceneProxy {
        std::function<void()> update;
        std::function<void()> render;
        std::function<void()> enter;
        std::function<void()> leave;
    };

    struct EnumlatorState {
        float pixel_size;
        std::unordered_map<Scene, SceneProxy> scenes;
        SceneProxy scene;
    };

    bool emulator_initialize();
    
    void emulator_run();

    void emulator_quit();

}