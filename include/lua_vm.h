#pragma once

#include <string>

extern "C" {
#include <Lua/lua-5.4.7/include/lauxlib.h>
#include <Lua/lua-5.4.7/include/lua.h>
#include <Lua/lua-5.4.7/include/lualib.h>
}

namespace t8y {

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
            } else {
                if constexpr (std::is_integral_v<T>) {
                    return static_cast<T>(lua_tonumber(state, n));
                }
                if constexpr (std::is_floating_point_v<T>) {
                    return static_cast<T>(lua_tonumber(state, n));
                }
                if constexpr (std::is_same_v<T, std::string>) {
                    return std::string(lua_tostring(state, n));
                }
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
            } else {
                if constexpr (std::is_integral_v<T>) {
                    return lua_isnumber(state, N + 1) && is<N + 1, Args...>();
                }
                if constexpr (std::is_floating_point_v<T>) {
                    return lua_isnumber(state, N + 1) && is<N + 1, Args...>();
                }
                if constexpr (std::is_same_v<T, std::string>) {
                    return lua_isstring(state, N + 1) && is<N + 1, Args...>();
                }
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
            } else {
                if constexpr (std::is_integral_v<T>) {
                    return lua_isnumber(state, n);
                }
                if constexpr (std::is_floating_point_v<T>) {
                    return lua_isnumber(state, n);
                }
                if constexpr (std::is_same_v<T, std::string>) {
                    return lua_isstring(state, n);
                }
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
            } else {
                if constexpr (std::is_integral_v<T>) {
                    lua_pushnumber(state, val);
                }
                if constexpr (std::is_floating_point_v<T>) {
                    lua_pushnumber(state, val);
                }
                if constexpr (std::is_same_v<T, std::string>) {
                    lua_pushstring(state, val.c_str());
                }
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

    template <typename Cls>
    class LuaVM {
    private:
        typedef int (Cls::*ClsFn)(const VMCall &call);

    public:
        template <ClsFn Fn>
        static int dispatch(lua_State *l) {
            auto cls = *static_cast<Cls **>(lua_getextraspace(l));
            auto call = VMCall(l);
            return ((*cls).*Fn)(call);
        }

        LuaVM() = default;

        void clear() {
            if (m_state) {
                lua_close(m_state);
                m_state = nullptr;
            }
        }

        bool reset(const std::string &src, Cls *cls) {
            clear();

            m_state = luaL_newstate();

            auto ret = luaL_loadstring(m_state, src.data());

            if (ret) {
                m_err = lua_tostring(m_state, -1);
                return false;
            }

            luaL_openlibs(m_state);

            *static_cast<Cls **>(lua_getextraspace(m_state)) = cls;

            lua_pcall(m_state, 0, 0, 0);

            return true;
        }

        template <ClsFn Fn>
        void bind(const std::string &fn_name) {
            lua_pushcclosure(m_state, &dispatch<Fn>, 0);
            lua_setglobal(m_state, fn_name.c_str());
        }

        template <typename... Args>
        bool call(const std::string &fn_name, const Args &...args) {
            auto i = lua_getglobal(m_state, fn_name.c_str());
            pushArgs(args...);
            auto ret = lua_pcall(m_state, 0, 0, 0);
            if (ret) {
                m_err = lua_tostring(m_state, -1);
                return false;
            }
            return true;
        }

        bool isFn(const std::string &fn_name) {
            auto i = lua_getglobal(m_state, fn_name.c_str());
            auto is = lua_isfunction(m_state, -1);
            lua_pop(m_state, -1);
            return i && is;
        }

        const auto &err() {
            return m_err;
        }

    private:
        void pushArgs() {

        };

        template <typename T, typename... Args>
        void pushArgs(const T &val, const Args &...args) {
            if constexpr (std::is_integral_v<T>) {
                lua_pushnumber(m_state, val);
            }
            if constexpr (std::is_floating_point_v<T>) {
                lua_pushnumber(m_state, val);
            }
            if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(m_state, val.c_str());
            }
            if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(m_state, val);
            }

            pushArgs(args...);
        }

    private:
        lua_State *m_state{nullptr};

        std::string m_err;
    };
}