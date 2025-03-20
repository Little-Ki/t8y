#include "t8_emulator.h"
#include "t8_font.h"
#include "t8_gamepad.h"
#include "t8_graphic.h"
#include "t8_keybd.h"
#include "t8_mouse.h"
#include "t8_signal.h"
#include "t8_sinput.h"
#include "t8_vscreen.h"
#include "t8_window.h"

#include "t8_console.h"
#include "t8_editor.h"
#include "t8_executor.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <thread>

namespace t8 {
    EnumlatorState state;

    void emulator_setup_screen() {
        vscreen_initialize(128, 128, state.pixel_size);
        vscreen_palette(0, 0, 0, 0);
        vscreen_palette(1, 250, 250, 250);
        vscreen_palette(2, 190, 190, 190);
        vscreen_palette(3, 200, 60, 90);
        vscreen_palette(4, 240, 130, 90);
        vscreen_palette(5, 250, 200, 120);
        vscreen_palette(6, 60, 180, 100);
        vscreen_palette(7, 40, 110, 120);
        vscreen_palette(8, 40, 70, 150);
        vscreen_palette(9, 150, 60, 150);
        vscreen_palette(10, 250, 120, 170);
        vscreen_palette(11, 250, 200, 170);
        vscreen_palette(12, 130, 140, 160);
        vscreen_palette(13, 150, 180, 190);
        vscreen_palette(14, 90, 110, 130);
        vscreen_palette(15, 50, 60, 90);
    }

    void emulator_setup_fonts() {
        int w, h, n;
        auto data = stbi_load_from_memory(font_data, sizeof(font_data), &w, &h, &n, 0);
        for (auto y = 0; y < std::min(128, h); y++) {
            for (auto x = 0; x < std::min(128, w); x++) {
                auto idx = y * w + x;
                if (data[idx]) {
                    graphic_font(x, y, true, true);
                    graphic_font(x, y, true, false);
                }
            }
        }

        stbi_image_free(data);
    }

    void emulator_swap_scene(Scene next) {
        if (state.scene) {
            state.scene->leave();
        }
        state.scene = &state.scenes[next];
        if (state.scene) {
            state.scene->enter();
        }
    }

    void emulator_setup_scene() {
        state.scenes[Scene::Console] = {
            console_update,
            console_draw,
            console_enter,
            console_leave};

        state.scenes[Scene::Editor] = {
            editor_update,
            editor_draw,
            editor_enter,
            editor_leave};

        state.scenes[Scene::Executor] = {
            executor_update,
            executor_draw,
            executor_enter,
            executor_leave};

        emulator_swap_scene(Scene::Console);
    }


    bool emulator_initialize() {
        state.pixel_size = 4;

        if (!window_initialize(128, 128, state.pixel_size)) {
            return false;
        }

        graphic_reset();

        emulator_setup_scene();
        emulator_setup_screen();
        emulator_setup_fonts();

        return true;
    }

    void emulator_handle_mouse(const SDL_Event &event) {

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const auto &i = event.motion;
            mouse_move(
                static_cast<uint8_t>(i.x / state.pixel_size),
                static_cast<uint8_t>(i.y / state.pixel_size));
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            const auto &i = event.button;
            mouse_button(i.button, true);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            const auto &i = event.button;
            mouse_button(i.button, false);
        }

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            const auto &i = event.wheel;
            mouse_wheel(static_cast<int8_t>(i.y));
        }
    }

    void emulator_handle_keybd(const SDL_Event &event) {
        if (event.type == SDL_EVENT_KEY_DOWN ||
            event.type == SDL_EVENT_KEY_UP) {
            const auto &i = event.key;
            if (i.scancode > 255)
                return;

            keybd_button(i.scancode, i.mod, i.repeat, event.type == SDL_EVENT_KEY_DOWN);
        }
    }

    void emulator_handle_gamepad(const SDL_Event &event) {
        if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
            gamepad_join(event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
            gamepad_remove(event.gdevice.which);
        }

        if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ||
            event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
            const auto &i = event.gbutton;
            gamepad_button(i.which, i.button, event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
        }
    }

    void emulator_handle_event(const SDL_Event &event) {
        switch (event.type) {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL: {
            emulator_handle_mouse(event);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            emulator_handle_keybd(event);
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            sinput_push(event.text.text);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            emulator_handle_gamepad(event);
            break;
        }
        }
    }

    void emulator_handle_scene() {
        if (state.scene) {
            state.scene->update();
            state.scene->draw();
        }
    }

    void emulator_handle_signal() {

        while (!signal_empty()) {
            const auto s = signal_peek();
            signal_pop();

            if (s.first == Signal::StartInput) {
                window_input(true);
            }
            if (s.first == Signal::StopInput) {
                window_input(false);
            }
            if (s.first == Signal::SwapEditor) {
                emulator_swap_scene(Scene::Editor);
            }
            if (s.first == Signal::SwapConsole) {
                emulator_swap_scene(Scene::Console);
            }
            if (s.first == Signal::SwapExecutor) {
                emulator_swap_scene(Scene::Executor);
            }
            if (s.first == Signal::Exception) {
                console_print(std::any_cast<std::string>(s.second), true);
                emulator_swap_scene(Scene::Console);
            }
            if (s.first == Signal::Print) {
                console_print(std::any_cast<std::string>(s.second), false);
            }
        }
    }

    void emulator_run() {

        SDL_Event event;

        while (true) {
            window_event(event);
            mouse_flush();
            keybd_flush();
            gamepad_flush();

            switch (event.type) {
                break;
            case SDL_EVENT_QUIT:
                return;
            default:
                emulator_handle_event(event);
            }

            emulator_handle_scene();

            for (auto y = 0; y < 128; y++) {
                for (auto x = 0; x < 128; x++) {
                    auto n = graphic_pixel(x, y);
                    vscreen_pixel(x, y, n);
                }
            }

            window_draw(vscreen_vertices(), vscreen_indices());

            emulator_handle_signal();

            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void emulator_quit() {
        window_quit();
    }
}