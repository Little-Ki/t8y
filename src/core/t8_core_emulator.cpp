#include "t8_core_emulator.h"
#include "t8_core_context.h"
#include "t8_core_memory.h"
#include "t8_font.h"
#include "t8_input_gamepad.h"
#include "t8_input_keyboard.h"
#include "t8_input_mouse.h"

#include "t8_core_painter.h"
#include "t8_core_window.h"

#include "t8_constants.h"

#include "t8_scene.h"
#include "t8_scene_console.h"
#include "t8_scene_editor.h"
#include "t8_scene_executor.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <thread>

using namespace t8::scene;
using namespace t8::input;

namespace t8::core {
    void emulator_setup_screen() {
        painter_palette(0, 0, 0, 0, 255);
        painter_palette(1, 250, 250, 250, 255);
        painter_palette(2, 190, 190, 190, 255);
        painter_palette(3, 200, 60, 90, 255);
        painter_palette(4, 240, 130, 90, 255);
        painter_palette(5, 250, 200, 120, 255);
        painter_palette(6, 60, 180, 100, 255);
        painter_palette(7, 40, 110, 120, 255);
        painter_palette(8, 40, 70, 150, 255);
        painter_palette(9, 150, 60, 150, 255);
        painter_palette(10, 250, 120, 170, 255);
        painter_palette(11, 250, 200, 170, 255);
        painter_palette(12, 130, 140, 160, 255);
        painter_palette(13, 150, 180, 190, 255);
        painter_palette(14, 90, 110, 130, 255);
        painter_palette(15, 50, 60, 90, 255);
    }

    void emulator_setup_fonts() {
        int w, h, n;
        auto data = stbi_load_from_memory(font_data, sizeof(font_data), &w, &h, &n, 0);
        for (auto y = 0; y < std::min(128, h); y++) {
            for (auto x = 0; x < std::min(128, w); x++) {
                auto idx = y * w + x;
                if (data[idx]) {
                    painter_font(x, y, true, true);
                    painter_font(x, y, true, false);
                }
            }
        }

        stbi_image_free(data);
    }

    void emulator_setup_scene() {
        scene_register(SCENE_ID_CONSOLE, {console_update,
                                          console_draw,
                                          console_enter,
                                          console_leave});

        scene_register(SCENE_ID_EDITOR, {editor_update,
                                         editor_draw,
                                         editor_enter,
                                         editor_leave});

        scene_register(SCENE_ID_EXECUTOR,
                       {executor_update,
                        executor_draw,
                        executor_enter,
                        executor_leave});

        scene_swap(SCENE_ID_CONSOLE);
    }

    bool emulator_initialize() {
        if (!window_initialize(128, 128, 4)) {
            return false;
        }

        painter_reset();

        emulator_setup_scene();
        emulator_setup_screen();
        emulator_setup_fonts();

        return true;
    }

    void emulator_handle_mouse(const SDL_Event &event) {

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            const auto &i = event.motion;
            mouse_move(
                static_cast<int16_t>(i.x / 4),
                static_cast<int16_t>(i.y / 4));
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

            keyboard_button(i.scancode, i.mod, i.repeat, event.type == SDL_EVENT_KEY_DOWN);
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
            ctx_inputs().push(event.text.text);
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

    void emulator_handle_signal() {

        while (!ctx_signals().empty()) {
            const auto s = ctx_signals().front();
            ctx_signals().pop();
            
            if (s.type == SIGNAL_START_INPUT) {
                window_input(true);
            }
            if (s.type == SIGNAL_STOP_INPUT) {
                window_input(false);
            }
            if (s.type == SIGNAL_SWAP_EDITOR) {
                scene_swap(SCENE_ID_EDITOR);
            }
            if (s.type == SIGNAL_SWAP_CONSOLE) {
                scene_swap(SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_SWAP_EXECUTOR) {
                scene_swap(SCENE_ID_EXECUTOR);
            }
            if (s.type == SIGNAL_EXCEPTION) {
                console_print(std::get<std::string>(s.value), true);
                scene_swap(SCENE_ID_CONSOLE);
            }
            if (s.type == SIGNAL_PRINT) {
                console_print(std::get<std::string>(s.value), false);
            }
        }
    }

    void emulator_run() {

        SDL_Event event;

        while (true) {
            window_event(event);
            mouse_flush();
            keyboard_flush();
            gamepad_flush();

            switch (event.type) {
                break;
            case SDL_EVENT_QUIT:
                return;
            default:
                emulator_handle_event(event);
            }

            scene_update();
            scene_draw();
            uint32_t buffer[128 * 128];

            for (auto y = 0; y < 128; y++) {
                for (auto x = 0; x < 128; x++) {
                    auto n = painter_pixel(x, y);
                    n = painter_palette(n);
                    auto color = memory()->palette[n];
                    buffer[y * 128 + x] = color;
                }
            }

            window_draw(buffer);

            emulator_handle_signal();

            std::this_thread::sleep_for(std::chrono::milliseconds(3));
        }
    }

    void emulator_quit() {
        window_quit();
    }
}