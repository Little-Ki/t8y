#include "t8_vm.h"

namespace t8 {

    VMState state;

    int vm_call_clear(const VMCall &call) {
        return 0;
    }

    template <typename F>
    static int vm_dispatcher(lua_State *l) {
        auto call = VMCall(l);
        return F(call);
    }

    template <typename F>
    static void vm_bind_call(const char* fn_name) {
        lua_pushcclosure(state.vm, &vm_dispatcher<F>, 0);
        lua_setglobal(state.vm, fn_name);
    }

    bool vm_initialize(const std::string &src) {
        vm_clear();

        state.vm = luaL_newstate();

        auto ret = luaL_loadstring(state.vm, src.data());

        if (ret) {
            state.err_msg = lua_tostring(state.vm, -1);
            return false;
        }

        luaL_openlibs(state.vm);

        lua_pcall(state.vm, 0, 0, 0);

        // vm_bind_call<&vm_call_clear>("clear");

        return true;
    }

    void vm_clear() {
        if (state.vm) {
            lua_close(state.vm);
            state.vm = nullptr;
        }
    }

    const std::string &vm_error() {
        return state.err_msg;
    }

    bool vm_execute(const char *fn_name) {
        if (!state.vm)
            return false;

        auto p = lua_getglobal(state.vm, fn_name);
        auto i = lua_isfunction(state.vm, -1);

        if (!(p && i)) {
            lua_pop(state.vm, -1);
            return true;
        } else {
            auto ret = lua_pcall(state.vm, 0, 0, 0);
            if (ret) {
                state.err_msg = lua_tostring(state.vm, -1);
                return false;
            }
            return true;
        }
    }

}