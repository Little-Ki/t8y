#pragma once
#include <memory>
#include <unordered_map>
#include <any>

#include "virtual_screen.h"
#include "window.h"

#include "scene.h"
#include "app.h"

namespace t8y {
    class SceneManager;

    class Emulator {
    public:
        Emulator();

        bool initialize();

        void run();

        void quit();

    protected:
        void setupFont();
        
        void setupScene();

        void setupScreen();

        void handleInput(const SDL_Event &event);

        void handleSignal();

        void handleKeyboard(const SDL_Event &event);

        void handleMouse(const SDL_Event &event);

        void handleGamepad(const SDL_Event &event);

    private:
        std::shared_ptr<Window> m_window;
        std::shared_ptr<VirtualScreen> m_screen;

        std::shared_ptr<App> m_app;

        SceneManager m_scene;

    };
}