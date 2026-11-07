#pragma once
#include <functional>
#include <unordered_map>
#include <memory>

#include "core_context.h"
#include "core_window.h"
#include "scene_console.h"

namespace t8::core
{
	using namespace t8::core::window;
	using namespace t8::scene::console;

	class Emulator
	{
	public:
		Emulator();
		
		bool init();
		void run();
		void quit();

	private:
		void scene_update();
		void scene_draw();
		void scene_enter();
		void scene_leave();
		void scene_swap(uint16_t next);

		void setup_fonts();
		void setup_palette();

		void handle_signal();

		void handle_event(const SDL_Event &event);
		void handle_mouse(const SDL_Event &event);
		void handle_keybd(const SDL_Event &event);
		void handle_gamepad(const SDL_Event &event);

	private:
		std::unique_ptr<AppContext> _ctx;
		std::unique_ptr<uint32_t[]> _buffer;
		std::unique_ptr<WindowState> _window;
		std::unique_ptr<ConsoleState> _console;

		uint32_t _pixel_size = 3;
		uint16_t _scene;
	};
}