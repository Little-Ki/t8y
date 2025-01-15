#pragma once

#include "lua_vm.h"
#include "scene.h"
#include "timer.h"

namespace t8y {

    class ExecutionView : public IScene {
    private:
        using VM = LuaVM<ExecutionView>;

    public:
        ExecutionView(App *app);

        virtual void update();

        virtual void draw();

        virtual void enter();

        virtual void leave();

        ~ExecutionView();

    private:
        bool resetVM();

    private:
        int clip(const VMCall &call);
        int pal(const VMCall &call);
        int palt(const VMCall &call);
        int cls(const VMCall &call);
        int camera(const VMCall &call);
        int pget(const VMCall &call);
        int pset(const VMCall &call);
        int fget(const VMCall &call);
        int fset(const VMCall &call);
        int sget(const VMCall &call);
        int sset(const VMCall &call);
        int mget(const VMCall &call);
        int mset(const VMCall &call);

        int line(const VMCall &call);
        int circ(const VMCall &call);
        int rect(const VMCall &call);

        int print(const VMCall &call);
        int spr(const VMCall &call);
        int map(const VMCall &call);
        int btn(const VMCall &call);
        int btnp(const VMCall &call);
        int key(const VMCall &call);
        int keyp(const VMCall &call);
        int mouse(const VMCall &call);
        int log(const VMCall &call);
        int time(const VMCall &call);
        int tstamp(const VMCall &call);

    private:
        VM m_vm;

        Timer m_timer;
    };

}