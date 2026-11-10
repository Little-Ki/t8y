#include "scene_executor.h"
#include "core_context.h"
#include "core_gfx.h"

#include "input_keyboard.h"
#include "input_mouse.h"
#include "input_gamepad.h"

#include "constants.h"

#include <algorithm>

using namespace t8::input;

#define ASSERT_EXECUTE(ctx, x)                                 \
    {                                                          \
        auto r = (x);                                          \
        if (!r.valid())                                        \
        {                                                      \
            sol::error err = r;                                \
            ctx->signals.push({SIGNAL_EXCEPTION, err.what()}); \
            return;                                            \
        }                                                      \
    }

namespace t8::scene::executor
{
    static void lua_binding(AppContext *ctx, sol::state &lua)
    {
        auto mem = ctx->memory;

        lua.set_function(
            "clip",
            sol::overload(
                [=](int x, int y, int w, int h)
                { gfx::set_clip(mem, x, y, w, h); },
                [=]()
                { gfx::set_clip(mem); }));

        lua.set_function(
            "pal",
            sol::overload(
                [=](uint8_t a, uint8_t b)
                { gfx::set_pmap(mem, a, b); },
                [=](uint8_t a)
                { return gfx::get_pmap(mem, a); }));

        lua.set_function(
            "palt",
            sol::overload(
                [=](uint8_t c, bool b)
                { gfx::set_trans(mem, c, b); },
                [=](std::optional<uint16_t> m)
                { gfx::set_trans(mem, m.value_or(0)); }));

        lua.set_function(
            "cls",
            [=](std::optional<uint8_t> c)
            { gfx::clear(mem, c.value_or(0)); });

        lua.set_function(
            "pget",
            [=](int x, int y)
            { return gfx::get_pixel(mem, x, y); });

        lua.set_function(
            "pset",
            [=](int x, int y, uint8_t c)
            { gfx::set_pixel(mem, x, y, c); });

        lua.set_function(
            "fget",
            [=](uint8_t i)
            { gfx::get_flags(mem, i); });

        lua.set_function(
            "fset",
            [=](uint8_t i, uint8_t f)
            { gfx::set_flags(mem, i, f); });

        lua.set_function(
            "sget",
            [=](int x, int y)
            { return gfx::get_sprite(mem, x, y); });

        lua.set_function(
            "sset",
            [=](int x, int y, uint8_t c)
            { gfx::set_sprite(mem, x, y, c); });

        lua.set_function(
            "mget",
            [=](int x, int y)
            { return gfx::get_tilemap(mem, x, y); });

        lua.set_function(
            "mset",
            [=](int x, int y, uint8_t i)
            { gfx::set_tilemap(mem, x, y, i); });

        lua.set_function(
            "line",
            [=](int x0, int y0, int x1, int y1, uint8_t c)
            { gfx::draw_line(mem, x0, y0, x1, y1, c); });

        lua.set_function(
            "cric",
            [=](int x, int y, int r, int c, std::optional<bool> f)
            {
                gfx::draw_circle(mem, x, y, r, c, f.value_or(false));
            });

        lua.set_function(
            "rect",
            [=](int x, int y, int w, int h, uint8_t c, std::optional<bool> f)
            {
                gfx::draw_rect(mem, x, y, w, h, c, f.value_or(false));
            });

        lua.set_function(
            "print",
            [=](
                std::string s, int x, int y,
                std::optional<uint8_t> _c,
                std::optional<int> _w0, std::optional<int> _w1)
            {
                const auto w0 = _w0.value_or(4);
                const auto w1 = _w1.value_or(8);
                const auto c = _c.value_or(1);
                const auto sx = x;
                for (auto &ch : s)
                {
                    if (ch == '\n')
                    {
                        y += 8;
                        x = sx;
                    }
                    else if (ch != '\r')
                    {
                        gfx::draw_char(mem, ch, x, y, c, true);
                        x += (static_cast<uint8_t>(ch) < 0x80) ? w0 : w1;
                    }
                }
            });

        lua.set_function(
            "btn",
            [=](uint8_t i)
            { return gamepad::down(ctx->gamepad, i >> 3, 1 << (i & 0b111)); });

        lua.set_function(
            "btnp",
            [=](uint8_t i)
            { return gamepad::pressed(ctx->gamepad, i >> 3, 1 << (i & 0b111)); });

        lua.set_function(
            "key",
            [=](uint8_t k)
            { return keyboard::down(ctx->keyboard, k); });

        lua.set_function(
            "keyp",
            [=](uint8_t k)
            { return keyboard::pressed(ctx->keyboard, k); });

        lua.set_function(
            "mouse",
            [=]()
            {
                auto mouse = &ctx->mouse;
                return std::make_tuple(mouse->x, mouse->y, mouse->z, mouse->current);
            });

        lua.set_function(
            "log",
            [=](std::string s)
            { ctx->signals.push({SIGNAL_PRINT, s}); });

        lua.set_function(
            "time",
            [=]()
            { return ctx->timer.ticks(); });

        lua.set_function(
            "tstamp",
            []()
            { return std::chrono::seconds(std::time(NULL)); });

        lua.set_function(
            "spr",
            [=](uint8_t id, int x, int y, std::optional<int> _s, std::optional<int> _f, std::optional<int> _r)
            {
                const auto s = _s.value_or(0);
                const auto f = _f.value_or(0);
                const auto r = _r.value_or(0);
                const auto ps = std::clamp(s, 1, 4);

                auto sprite_x = (id & 0xF) << 3;
                auto sprite_y = ((id >> 4) & 0xF) << 3;

                for (auto dy = 0; dy < 8; dy++)
                {
                    for (auto dx = 0; dx < 8; dx++)
                    {
                        auto tx = dx;
                        auto ty = dy;

                        if (f & 0b1)
                            tx = 7 - tx;

                        if (f & 0b10)
                            ty = 7 - ty;

                        if (r & 0b1)
                        {
                            auto t = x;
                            tx = ty;
                            ty = 7 - t;
                        }

                        if (r & 0b10)
                        {
                            tx = 7 - tx;
                            ty = 7 - ty;
                        }

                        auto color = gfx::get_sprite(mem, sprite_x + tx, sprite_y + ty);

                        gfx::draw_rect(mem, x + dx * ps, y + dy * ps, ps, ps, color, true);
                    }
                }
            });

        lua.set_function(
            "map",
            [=](int mx, int my, int mw, int mh, int sx, int sy, std::optional<int> _scale, std::optional<uint8_t> _layers)
            {
                const auto scale = std::clamp(_scale.value_or(1), 1, 4);
                const auto layers = _layers.value_or(0xff);
                const auto chunk_size = 8 * scale;
                const auto pixel_size = scale;

                if (mx < 0)
                {
                    sx -= mx * chunk_size;
                    mw += mx;
                    mx = 0;
                }

                if (my < 0)
                {
                    sy -= my * chunk_size;
                    mh += my;
                    my = 0;
                }

                if (mw < 0 || mh < 0)
                    return;

                auto l = std::clamp(mx, 0, 128);
                auto t = std::clamp(my, 0, 128);
                auto r = std::clamp(mx + mw, 0, 128);
                auto b = std::clamp(my + mh, 0, 128);

                for (auto y = t; y < b; y++)
                {
                    for (auto x = l; x < r; x++)
                    {
                        auto id = gfx::get_tilemap(mem, x, y);

                        if (!(id & layers))
                            continue;

                        auto sprite_x = (id & 0xF) << 3;
                        auto sprite_y = ((id >> 4) & 0xF) << 3;

                        for (auto dy = 0; dy < 8; dy++)
                        {
                            for (auto dx = 0; dx < 8; dx++)
                            {
                                auto color = gfx::get_sprite(mem, sprite_x + dx, sprite_y + dy);
                                gfx::draw_rect(
                                    mem,
                                    sx + x * chunk_size + dx * pixel_size,
                                    sy + y * chunk_size + dy * pixel_size,
                                    pixel_size, pixel_size, color, true);
                            }
                        }
                    }
                }
            });
    }

    void update(AppContext *ctx, ExecutorState *state)
    {
        if (keyboard::pressed(ctx->keyboard, SCANCODE_ESC) && !state->paused)
        {
            state->paused = true;
            state->select = 0;
        }

        if (state->paused)
        {
            if (keyboard::pressed(ctx->keyboard, SCANCODE_UP) && state->select > 0)
            {
                state->select -= 1;
            }
            if (keyboard::pressed(ctx->keyboard, SCANCODE_DOWN))
            {
                state->select = std::clamp(state->select + 1, 0, 1);
            }
            if (keyboard::pressed(ctx->keyboard, 40) || keyboard::pressed(ctx->keyboard, 88))
            {
                state->paused = false;
                if (state->select == 1)
                {
                    ctx->signals.push({SIGNAL_SWAP_CONSOLE});
                }
            }
        }

        if (!state->paused)
        {
            auto &vm = state->vm;
            if (vm->update)
            {
                ASSERT_EXECUTE(ctx, vm->update());
            }
        }
    }

    void draw(AppContext *ctx, ExecutorState *state)
    {
    }

    void enter(AppContext *ctx, ExecutorState *state)
    {
        auto &vm = state->vm;
        ctx->memory = &ctx->exec_memory;

        vm.emplace();
        vm->lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table);
        lua_binding(ctx, vm->lua);

        ASSERT_EXECUTE(ctx, vm->lua.safe_script(ctx->script, sol::script_pass_on_error));

        vm->init = vm->lua["init"];
        vm->update = vm->lua["update"];
        vm->draw = vm->lua["draw"];

        if (vm->init)
        {
            ASSERT_EXECUTE(ctx, vm->init());
        };

        gfx::clear(ctx->memory, 0);

        ctx->signals.push({SIGNAL_STOP_INPUT});
    }

    void leave(AppContext *ctx, ExecutorState *state)
    {
        auto &vm = state->vm;
        vm->lua.collect_garbage();
        vm.reset();
        ctx->memory = &ctx->base_memory;
    }
}