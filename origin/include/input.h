#pragma once

#include <memory>
#include <queue>
#include <string>

namespace t8y {

    struct AppContext;

    class MouseInput {
    public:
        MouseInput(AppContext* ctx) : ctx(ctx) {}

        void flush();
        bool pressed(uint8_t btn);
        bool released(uint8_t btn);
        bool clicked(int x, int y, int w, int h, uint8_t btn = 1);
        bool drag(int x, int y, int w, int h, uint8_t btn = 1);
        bool inside(int x, int y, int w, int h);
        void move(uint8_t x, uint8_t y);
        void button(uint8_t btn, bool down);
        bool down(uint8_t btn);
        void wheel(int8_t z);
        int8_t dx() const;
        int8_t dy() const;
        uint8_t x() const;
        uint8_t y() const;
        int8_t z() const;
        uint8_t button() const;

    private:
        struct MouseOb {
            int index, focus;
        };

        MouseOb m_ob[3]{};

        AppContext *ctx;
    };

    class KayboardInput {
    public:
        KayboardInput(AppContext *ctx) : ctx(ctx) {}

        void flush();
        void button(uint8_t btn, uint16_t mod, bool repeat, bool down);
        bool down(uint8_t btn);
        bool pressed(uint8_t btn);
        bool repeated(uint8_t btn);
        bool released(uint8_t btn);
        bool triggered(uint8_t btn);
        bool ctrl();
        bool shift();
        bool alt();
        bool capslock();

    private:
        AppContext *ctx;
    };

    class GamepadInput {
    public:
        GamepadInput(AppContext *ctx) : ctx(ctx) {}

        void flush();
        void add(uint32_t id);
        void remove(uint32_t id);
        void button(uint32_t id, uint8_t button, bool down);

    private:
        AppContext *ctx;

		uint32_t idMapper[4]{ 0 };
    };

    using TextInput = std::queue<std::string>;

}