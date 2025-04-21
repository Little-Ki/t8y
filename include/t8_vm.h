#pragma once

#include <string>

extern "C" {
#include <Lua/lua-5.4.7/include/lauxlib.h>
#include <Lua/lua-5.4.7/include/lua.h>
#include <Lua/lua-5.4.7/include/lualib.h>
}

#include "t8_timer.h"

namespace t8 {

    class VMCall {
    public:
        VMCall(lua_State *l) : state(l) {
            n = lua_gettop(l);
        };

        template <typename T = void>
        T get(int n) const {
            if (n > this->n)
                return T{};

            if constexpr (std::is_same_v<T, bool>) {
                return static_cast<T>(lua_toboolean(state, n));
            }
            if constexpr (std::is_integral_v<T>) {
                return static_cast<T>(lua_tonumber(state, n));
            }
            if constexpr (std::is_floating_point_v<T>) {
                return static_cast<T>(lua_tonumber(state, n));
            }
            if constexpr (std::is_same_v<T, std::string>) {
                return std::string(lua_tostring(state, n));
            }

            return T{};
        };

        template <int N = 0>
        bool is() const {
            return N == n;
        }

        template <int N, typename T, typename... Args>
        bool is() const {
            if (N > n)
                return false;

            if constexpr (std::is_same_v<T, bool>) {
                return lua_isboolean(state, N + 1) && is<N + 1, Args...>();
            }
            if constexpr (std::is_integral_v<T>) {
                return lua_isnumber(state, N + 1) && is<N + 1, Args...>();
            }
            if constexpr (std::is_floating_point_v<T>) {
                return lua_isnumber(state, N + 1) && is<N + 1, Args...>();
            }
            if constexpr (std::is_same_v<T, std::string>) {
                return lua_isstring(state, N + 1) && is<N + 1, Args...>();
            }

            return false;
        };

        template <typename... Args>
        bool is() const {
            return is<0, Args...>();
        };

        template <typename T>
        bool is(int n) const {
            if (n > this->n)
                return false;

            if constexpr (std::is_same_v<T, bool>) {
                return lua_isboolean(state, n);
            }
            if constexpr (std::is_integral_v<T>) {
                return lua_isnumber(state, n);
            }
            if constexpr (std::is_floating_point_v<T>) {
                return lua_isnumber(state, n);
            }
            if constexpr (std::is_same_v<T, std::string>) {
                return lua_isstring(state, n);
            }

            return false;
        }

        template <int N = 0>
        int ret() const {
            return N;
        }

        template <int N, typename T, typename... Args>
        int ret(const T &val, const Args &...args) const {

            if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(state, val);
            }
            if constexpr (std::is_integral_v<T>) {
                lua_pushnumber(state, val);
            }
            if constexpr (std::is_floating_point_v<T>) {
                lua_pushnumber(state, val);
            }
            if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(state, val.c_str());
            }

            return ret<N + 1>(args...);
        }

        template <typename... Args>
        int ret(const Args &...args) const {
            return ret<0, Args...>(args...);
        };

        const int &argc() const {
            return n;
        }

    private:
        int n;

        lua_State *state;
    };

    struct VMState {
        lua_State *vm{nullptr};
        std::string err_msg;
    };

    using vm_call_fn = int (*)(const VMCall &call);

    bool vm_initialize(const std::string &src);

    void vm_release();

    const std::string &vm_error();

    bool vm_execute(const char *fn_name);

}