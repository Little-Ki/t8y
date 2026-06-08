#pragma once
#include "t8_constants.h"
#include "text_editor/t8_text_editor.hpp"
#include <cstdint>
#include <sol2/include/sol/sol.hpp>

namespace t8 {

    struct VirtualMemory {
        uint8_t screen[0x2000];
        uint8_t sprite[0x2000];

        uint8_t map[0x4000];

        uint32_t palette[0xF0];
        uint8_t palette_mapping[0x8];
        uint16_t palette_mask;

        uint8_t flags[0x100];
        uint8_t default_font[0x800];
        uint8_t custom_font[0x800];

        uint8_t view_clip[4];
        int8_t draw_offset[2];

        uint32_t cache[256];
    };

    struct GamepadState {
        union {
            struct {
                bool up : 1;
                bool down : 1;
                bool left : 1;
                bool right : 1;
                bool a : 1;
                bool b : 1;
                bool x : 1;
                bool y : 1;
            };
            uint8_t button = 0;
        } down[4], pressed[4];

        uint32_t mapper[4]{0};
    };

    struct MouseState {
        int16_t x = 0, y = 0, z = 0;
        uint8_t buttons = 0;
        uint8_t pressed = 0;
        int16_t dx = 0, dy = 0;

        struct {
            int active_id, focus_id;
        } tracks[3];
    };

    struct KeyboardState {
        uint8_t button[32] = {0};
        uint8_t pressed[32] = {0};
        uint8_t repeated[32] = {0};
        uint8_t modifier = 0;
    };

    struct EditorState {
        struct Map {
            uint8_t sprite_id = 0;
            uint8_t zoom = 1;
            uint8_t page = 0;
            int x = 0;
            int y = 11;
        } map;

        struct Script {
            int scroll_x = 0;
            int scroll_y = 0;
            bool dragging = false;
            text_editor::TextEditor editor;
        } script;

        struct Sheet {
            uint8_t tool = t8::EDITOR_PENCIL;
            uint8_t sprite_id = 0;
            uint8_t color = 1;
            uint8_t page = 0;
            uint8_t zoom = 1;
            uint8_t pencil_size = 1;
            bool edit_font = false;
        } sheet;
    };

    struct ConsoleState {
        struct Record {
            int front_height;
            int height;
            bool prefix;
            uint8_t color;
            std::string text;
        };

        std::vector<std::string> history;
        size_t history_index = 0;
        bool use_history = false;

        std::vector<Record> records;
        std::string input;
        size_t cursor = 0;

        bool first_time = true;
    };

    struct ExecutorState {
        struct ScriptVM {
            sol::state lua;
            sol::protected_function init;
            sol::protected_function update;
            sol::protected_function draw;
        };

        uint8_t select;
        bool is_paused = false;

        std::optional<ScriptVM> vm;
    };

    struct AppContext;

    struct SceneState {
        struct Record {
            std::function<void(AppContext *)> update;
            std::function<void(AppContext *)> draw;
            std::function<void(AppContext *)> enter;
            std::function<void(AppContext *)> leave;
        };

        std::unordered_map<uint32_t, Record> scene_records;
        Record *current_scene = nullptr;
    };

    struct AppContext {
        VirtualMemory base_memory;
        VirtualMemory runtime_memory;
        VirtualMemory *active_memory = &base_memory;

        MouseState mouse_state;
        KeyboardState keyboard_state;
        GamepadState gamepad_state;
        EditorState editor_state;
        ConsoleState console_state;
        ExecutorState executor_state;

        SceneState scene_state;
    };
}