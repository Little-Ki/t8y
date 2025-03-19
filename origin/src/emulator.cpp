#include "emulator.h"
#include <any>
#include <bitset>
#include <chrono>
#include <functional>
#include <thread>

#include "console_view.h"
#include "edit_view.h"
#include "exec_view.h"
#include "font_png.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace t8y {

    Emulator::Emulator() : m_window(std::make_shared<Window>()),
                           m_screen(std::make_shared<VirtualScreen>()),
                           m_app(std::make_shared<App>()) {
    }

    bool Emulator::initialize() {
        if (!m_window->initialize(128, 128, 4)) {
            return false;
        }

        m_app->graphic.reset();

        setupScreen();
        setupScene();
        setupFont();

        return true;
    }

    void Emulator::run() {
        SDL_Event event;

        while (true) {
            m_window->pollEvent(event);
            m_app->mouse.flush();
            m_app->keyboard.flush();
            m_app->gamepad.flush();

            switch (event.type) {
                break;
            case SDL_EVENT_QUIT:
                return;
            default:
                handleInput(event);
            }

            m_scene.update();
            m_scene.draw();

            for (auto y = 0; y < 128; y++) {
                for (auto x = 0; x < 128; x++) {
                    auto n = m_app->graphic.getPixel(x, y);
                    m_screen->setPixel(x, y, n);
                }
            }

            m_window->draw(m_screen->vertices(), m_screen->indices());
               
            handleSignal();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void Emulator::quit() {
        m_window->quit();
    }

    void Emulator::setupFont() {
        int w, h, n;
        auto data = stbi_load_from_memory(font_png, sizeof(font_png), &w, &h, &n, 0);
        for (auto y = 0; y < std::min(128, h); y++) {
            for (auto x = 0; x < std::min(128, w); x++) {
                auto idx = y * w + x;
                if (data[idx]) {
                    m_app->graphic.setFont(x, y, true);
                    m_app->graphic.setFont(x, y, true, true);
                }
            }
        }

        stbi_image_free(data);
    }

    void Emulator::setupScene() {
        m_scene.emplace<EditView>(0, m_app.get());
        m_scene.emplace<ConsoleView>(1, m_app.get());
        m_scene.emplace<ExecutionView>(2, m_app.get());
        m_scene.swap(1);
    }

    void Emulator::setupScreen() {
        m_screen->initialize(128, 128, 4);
        m_screen->setPalette(0, 0, 0, 0);
        m_screen->setPalette(1, 250, 250, 250);
        m_screen->setPalette(2, 190, 190, 190);
        m_screen->setPalette(3, 200, 60, 90);
        m_screen->setPalette(4, 240, 130, 90);
        m_screen->setPalette(5, 250, 200, 120);
        m_screen->setPalette(6, 60, 180, 100);
        m_screen->setPalette(7, 40, 110, 120);
        m_screen->setPalette(8, 40, 70, 150);
        m_screen->setPalette(9, 150, 60, 150);
        m_screen->setPalette(10, 250, 120, 170);
        m_screen->setPalette(11, 250, 200, 170);
        m_screen->setPalette(12, 130, 140, 160);
        m_screen->setPalette(13, 150, 180, 190);
        m_screen->setPalette(14, 90, 110, 130);
        m_screen->setPalette(15, 50, 60, 90);
    }

    void Emulator::handleInput(const SDL_Event &event) {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL: {
            handleMouse(event);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_TEXT_INPUT: {
            handleKeyboard(event);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            handleGamepad(event);
            break;
        }
    }

    void Emulator::handleSignal() {
        while (!m_app->signal.empty()) {
            const auto signal = m_app->signal.back();
            const auto &s = signal.first;
            const auto &value = signal.second;
            m_app->signal.pop();

            if (s == SIGNAL_START_INPUT) {
                m_window->textInput(true);
            }
            if (s == SIGNAL_STOP_INPUT) {
                m_window->textInput(false);
            }
            if (s == SIGNAL_SWAP_EDIT) {
                m_scene.swap(0);
            }
            if (s == SIGNAL_SWAP_CONSOLE) {
                m_scene.swap(1);
            }
            if (s == SIGNAL_SWAP_EXEC) {
                m_scene.swap(2);
            }
            if (s == SIGNAL_EXCEPTION) {
                m_scene.get<ConsoleView>(1)
                    ->join(std::any_cast<std::string>(value));
                m_scene.swap(1);
            }
            if (s == SIGNAL_OUTPUT) {
                m_scene.get<ConsoleView>(1)
                    ->join(std::any_cast<std::string>(value));
                std::cout << std::any_cast<std::string>(value);
            }
        }
    }

    void Emulator::handleKeyboard(const SDL_Event &event) {
        if (event.type == SDL_EVENT_KEY_DOWN ||
            event.type == SDL_EVENT_KEY_UP) {
            const auto &i = event.key;
            if (i.scancode > 255)
                return;

            m_app->keyboard.button(
                i.scancode, i.mod, i.repeat,
                event.type == SDL_EVENT_KEY_DOWN);
        }

        if (event.type == SDL_EVENT_TEXT_INPUT) {
            const auto &i = event.text;
            m_app->texts.push(i.text);
        }
    }

    void Emulator::handleMouse(const SDL_Event &event) {
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const auto &i = event.motion;
            m_app->mouse.move(
                static_cast<uint8_t>(i.x / 4),
                static_cast<uint8_t>(i.y / 4));
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            const auto &i = event.button;
            m_app->mouse.button(i.button, true);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            const auto &i = event.button;
            m_app->mouse.button(i.button, false);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            const auto &i = event.wheel;
            m_app->mouse.wheel(static_cast<uint8_t>(i.y));
        }
    }

    void Emulator::handleGamepad(const SDL_Event &event) {

        if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
            const auto &i = event.gdevice;
            m_app->gamepad.add(i.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
            const auto &i = event.gdevice;
            m_app->gamepad.remove(i.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            const auto &i = event.gbutton;
            m_app->gamepad.button(
                i.which,
                i.button,
                event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
    }

}