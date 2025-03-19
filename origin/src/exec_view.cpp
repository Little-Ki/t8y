#include "exec_view.h"
#include "buffer.h"

#include <chrono>

namespace t8y {

    ExecutionView::ExecutionView(App *app) : IScene(app) {
    }

    void ExecutionView::update() {
        auto steps = m_timer.steps();
        if (m_vm.isFn("update") && steps < 3) {
            for (auto i = 0; i < steps; i++) {
                if (!m_vm.call("update")) {
                    app->signal.join(SIGNAL_EXCEPTION, m_vm.err());
                    return;
                };
            }
        }
        m_timer.consume(steps);
    }

    void ExecutionView::draw() {
        if (m_vm.isFn("draw")) {
            if (!m_vm.call("draw")) {
                app->signal.join(SIGNAL_EXCEPTION, m_vm.err());
                return;
            };
        }
    }

    void ExecutionView::enter() {
        m_timer.reset();

        if (!resetVM()) {
            return;
        }

        if (m_vm.isFn("init")) {
            if (!m_vm.call("init")) {
                app->signal.join(SIGNAL_EXCEPTION, m_vm.err());
                return;
            };
        }

        app->graphic.clear(0);
    }

    void ExecutionView::leave() {
        m_vm.clear();
    }

    ExecutionView::~ExecutionView() {
    }

    bool ExecutionView::resetVM() {

        if (!m_vm.reset(app->context.script, this)) {
            app->signal.join(SIGNAL_EXCEPTION, m_vm.err());
            return false;
        }

        m_vm.bind<&ExecutionView::clip>("clip");
        m_vm.bind<&ExecutionView::pal>("pal");
        m_vm.bind<&ExecutionView::palt>("palt");
        m_vm.bind<&ExecutionView::cls>("cls");
        m_vm.bind<&ExecutionView::camera>("camera");
        m_vm.bind<&ExecutionView::pget>("pget");
        m_vm.bind<&ExecutionView::pset>("pset");
        m_vm.bind<&ExecutionView::fget>("fget");
        m_vm.bind<&ExecutionView::fset>("fset");
        m_vm.bind<&ExecutionView::sget>("sget");
        m_vm.bind<&ExecutionView::sset>("sset");
        m_vm.bind<&ExecutionView::mget>("mget");
        m_vm.bind<&ExecutionView::mset>("mset");
        m_vm.bind<&ExecutionView::line>("line");
        m_vm.bind<&ExecutionView::circ>("circ");
        m_vm.bind<&ExecutionView::rect>("rect");
        m_vm.bind<&ExecutionView::print>("print");
        m_vm.bind<&ExecutionView::spr>("spr");
        m_vm.bind<&ExecutionView::map>("map");
        m_vm.bind<&ExecutionView::btn>("btn");
        m_vm.bind<&ExecutionView::btnp>("btnp");
        m_vm.bind<&ExecutionView::key>("key");
        m_vm.bind<&ExecutionView::keyp>("keyp");
        m_vm.bind<&ExecutionView::mouse>("mouse");
        m_vm.bind<&ExecutionView::log>("log");
        m_vm.bind<&ExecutionView::time>("time");
        m_vm.bind<&ExecutionView::tstamp>("tstamp");

        return true;
    }

    int ExecutionView::clip(const VMCall &call) {
        if (call.is<int, int, int, int>()) {
            app->graphic.clip(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4));
        }
        if (call.argc() == 0) {
            app->graphic.clip();
        }
        return 0;
    }

    int ExecutionView::pal(const VMCall &call) {
        if (call.is<int, int>()) {
            app->graphic.setPalt(call.get<int>(1), call.get<int>(2));
        }
        if (call.is<int>()) {
            auto p = app->graphic.getPalt(call.get<int>(1));
            return call.ret(p);
        }
        return 0;
    }

    int ExecutionView::palt(const VMCall &call) {
        if (call.is<int, bool>()) {
            app->graphic.setTrans(
                call.get<int>(1),
                call.get<bool>(2));
        }
        if (call.is<uint16_t>()) {
            app->graphic.setTrans(
                call.get<uint16_t>(1));
        }
        return 0;
    }

    int ExecutionView::cls(const VMCall &call) {
        if (call.argc() == 0) {
            app->graphic.clear(0);
        }
        if (call.is<uint8_t>()) {
            app->graphic.clear(call.get<uint8_t>(1));
        }
        return 0;
    }

    int ExecutionView::camera(const VMCall &call) {
        if (call.is<int8_t, int8_t>()) {
            app->graphic.camera(
                call.get<int8_t>(1),
                call.get<int8_t>(2));
        }
        return 0;
    }

    int ExecutionView::pget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                app->graphic.getPixel(
                    call.get<int>(1),
                    call.get<int>(2)));
        }

        return 0;
    }

    int ExecutionView::pset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            app->graphic.setPixel(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int ExecutionView::fget(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(
                app->graphic.getFlag(call.get<uint8_t>(1)));
        }
        return 0;
    }

    int ExecutionView::fset(const VMCall &call) {
        if (call.is<uint8_t, uint8_t>()) {
            app->graphic.setFlag(
                call.get<uint8_t>(1),
                call.get<uint8_t>(2));
        }
        return 0;
    }

    int ExecutionView::sget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                app->graphic.getSprite(
                    call.get<int>(1),
                    call.get<int>(2)));
        }
        return 0;
    }

    int ExecutionView::sset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            app->graphic.setSprite(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int ExecutionView::mget(const VMCall &call) {
        if (call.is<int, int>()) {
            return call.ret(
                app->graphic.getMap(
                    call.get<int>(1),
                    call.get<int>(2)));
        }

        return 0;
    }

    int ExecutionView::mset(const VMCall &call) {
        if (call.is<int, int, uint8_t>()) {
            app->graphic.setMap(
                call.get<int>(1),
                call.get<int>(2),
                call.get<uint8_t>(3));
        }
        return 0;
    }

    int ExecutionView::line(const VMCall &call) {
        if (call.is<int, int, int, int, uint8_t>()) {
            app->graphic.drawLine(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5));
        }
        return 0;
    }

    int ExecutionView::circ(const VMCall &call) {
        if (call.is<int, int, int, uint8_t, bool>()) {
            app->graphic.drawCircel(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<uint8_t>(4),
                call.get<bool>(5));
        }
        if (call.is<int, int, int, uint8_t>()) {
            app->graphic.drawCircel(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<uint8_t>(4));
        }
        return 0;
    }

    int ExecutionView::rect(const VMCall &call) {

        if (call.is<int, int, int, int, uint8_t>()) {
            app->graphic.drawRect(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5));
        }

        if (call.is<int, int, int, int, uint8_t, bool>()) {
            app->graphic.drawRect(
                call.get<int>(1),
                call.get<int>(2),
                call.get<int>(3),
                call.get<int>(4),
                call.get<uint8_t>(5),
                call.get<bool>(6));
        }

        return 0;
    }

    int ExecutionView::print(const VMCall &call) {
        if (call.argc() < 3) {
            return 0;
        }

        auto st = call.is<std::string>(1) ? call.get<std::string>(1) : std::string();
        auto bx = call.is<int>(2) ? call.get<int>(2) : 0;
        auto by = call.is<int>(3) ? call.get<int>(3) : 0;
        auto co = call.is<uint8_t>(4) ? call.get<uint8_t>(4) : 1;
        auto w0 = call.is<int>(5) ? call.get<int>(5) : 4;
        auto w1 = call.is<int>(6) ? call.get<int>(6) : 8;

        w0 = std::clamp(w0, 1, 8);
        w1 = std::clamp(w1, 1, 8);

        if (!st.empty()) {
            auto x = bx;
            auto y = by;

            for (const auto &ch : st) {
                if (ch == '\n') {
                    y += 8;
                    x = bx;
                } else if (ch != '\r') {
                    app->graphic.drawChar(ch, x, y, co, true);
                    x += (ch & 0x80) ? w1 : w0;
                }
            }
        }

        return 0;
    }

    int ExecutionView::spr(const VMCall &call) {
        if (call.argc() < 3) {
            return 0;
        }

        auto id = call.is<uint8_t>(1) ? call.get<uint8_t>(1) : 0;
        auto x = call.is<int>(2) ? call.get<int>(2) : 0;
        auto y = call.is<int>(3) ? call.get<int>(3) : 0;
        auto scale = call.is<int>(3) ? call.get<int>(3) : 1;
        auto flip = call.is<int>(4) ? call.get<int>(4) : 1;
        auto rotate = call.is<int>(5) ? call.get<int>(5) : 1;
        const auto pixelSize = std::clamp(scale, 1, 4);

        auto spX = (id & 0xF) << 3;
        auto spY = ((id >> 4) & 0xF) << 3;

        const auto flip_fn = [&](int &x, int &y) {
            if (flip & 0b1) {
                x = 7 - x;
            }
            if (flip & 0b10) {
                y = 7 - y;
            }
        };

        const auto rotate_fn = [&](int &x, int &y) {
            if (flip & 0b1) {
                auto tx = x;
                x = y;
                y = 7 - tx;
            }
            if (flip & 0b10) {
                x = 7 - x;
                y = 7 - y;
            }
        };

        for (auto inY = 0; inY < 8; inY++) {
            for (auto inX = 0; inX < 8; inX++) {
                auto tx = x;
                auto ty = y;
                auto color = app->graphic.getSprite(spX + tx, spY + ty);

                flip_fn(tx, ty);
                rotate_fn(tx, ty);

                app->graphic.drawRect(
                    x + inX * pixelSize,
                    y + inY * pixelSize,
                    pixelSize,
                    pixelSize,
                    color,
                    true);
            }
        }

        return 0;
    }

    int ExecutionView::map(const VMCall &call) {
        if (call.argc() < 6) {
            return 0;
        }

        auto mx = call.is<int>(1) ? call.get<int>(1) : 0;
        auto my = call.is<int>(2) ? call.get<int>(2) : 0;
        auto mw = call.is<int>(3) ? call.get<int>(3) : 1;
        auto mh = call.is<int>(4) ? call.get<int>(4) : 1;
        auto sx = call.is<int>(5) ? call.get<int>(5) : 0;
        auto sy = call.is<int>(6) ? call.get<int>(6) : 0;
        auto scale = call.is<int>(7) ? call.get<int>(7) : 1;
        const auto layers = call.is<uint8_t>(8) ? call.get<int>(8) : 0xFF;

        scale = std::clamp(scale, 1, 4);

        const auto chunkSize = 8 * scale;
        const auto pixelSize = scale;

        if (mx < 0) {
            sx -= mx * chunkSize;
            mw += mx;
            mx = 0;
        }

        if (my < 0) {
            sy -= my * chunkSize;
            mh += my;
            my = 0;
        }

        if (mw < 0 || mh < 0)
            return 0;

        auto l = std::clamp(mx, 0, 128);
        auto t = std::clamp(my, 0, 128);
        auto r = std::clamp(mx + mw, 0, 128);
        auto b = std::clamp(my + mh, 0, 128);

        for (auto y = t; y < b; y++) {
            for (auto x = l; x < r; x++) {
                auto id = app->graphic.getMap(x, y);

                if (!(id & layers))
                    continue;

                auto spX = (id & 0xF) << 3;
                auto spY = ((id >> 4) & 0xF) << 3;

                for (auto inY = 0; inY < 8; inY++) {
                    for (auto inX = 0; inX < 8; inX++) {
                        auto color = app->graphic.getSprite(spX + inX, spY + inY);
                        app->graphic.drawRect(
                            sx + x * chunkSize + inX * pixelSize,
                            sy + y * chunkSize + inY * pixelSize,
                            pixelSize,
                            pixelSize,
                            color,
                            true);
                    }
                }
            }
        }

        return 0;
    }

    int ExecutionView::btn(const VMCall &call) {
        if (call.is<uint8_t>()) {
            _1BitBuffer<32, 1> t(reinterpret_cast<uint8_t *>(app->context.gamepad.p));
            return call.ret(
                t.get(call.get<uint8_t>(1), 0));
        }

        return 0;
    }

    int ExecutionView::btnp(const VMCall &call) {
        if (call.is<uint8_t>()) {
            _1BitBuffer<32, 1> t(reinterpret_cast<uint8_t *>(app->context.gamepad.pressed));
            return call.ret(
                t.get(call.get<uint8_t>(1), 0));
        }

        return 0;
    }

    int ExecutionView::key(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(
                app->keyboard.down(
                    call.get<uint8_t>(1)));
        }
        return 0;
    }

    int ExecutionView::keyp(const VMCall &call) {
        if (call.is<uint8_t>()) {
            return call.ret(
                app->keyboard.pressed(
                    call.get<uint8_t>(1)));
        }
        return 0;
    }

    int ExecutionView::mouse(const VMCall &call) {
        const auto &m = app->context.mouse;
        return call.ret(
            m.x, m.y, m.z, m.button);
    }

    int ExecutionView::log(const VMCall &call) {
        if (call.is<std::string>()) {
            app->signal.join(SIGNAL_OUTPUT, call.get<std::string>(1));
        }
        return 0;
    }

    int ExecutionView::time(const VMCall &call) {
        return call.ret(m_timer.ticks());
    }

    int ExecutionView::tstamp(const VMCall &call) {
        auto unix_timestamp = std::chrono::seconds(std::time(NULL));
        return call.ret(unix_timestamp);
    }

}