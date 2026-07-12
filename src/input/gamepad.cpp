#include "input/gamepad.h"

#include <algorithm>

namespace t8::input
{

    void g_flush(GamepadState &s)
    {
        s.previous[0] = s.current[0];
        s.previous[1] = s.current[1];
        s.previous[2] = s.current[2];
        s.previous[3] = s.current[3];
    }

    void g_join(GamepadState &s, uint32_t id)
    {
        for (auto &i : s.mapper)
        {
            if (i == 0)
            {
                i = id;
                return;
            }
        }
    }

    void g_remove(GamepadState &s, uint32_t id)
    {
        for (auto &i : s.mapper)
        {
            if (i == id)
            {
                i = 0;
                break;
            }
        }

        for (auto i = 0; i < 4; i++)
        {
            if (s.mapper[i] != 0)
                continue;

            for (auto j = i + 1; j < 4; j++)
            {
                if (s.mapper[j] == 0)
                    continue;
                std::swap(s.mapper[i], s.mapper[j]);
                return;
            }
        }
    }

    void g_button(GamepadState &s, uint32_t id, uint8_t btn, bool down)
    {
        for (auto i = 0; i < 4; i++)
        {
            if (s.mapper[i] == id)
            {
                down ? s.current[i].btn |= btn : s.current[i].btn &= ~btn;
                return;
            }
        }
    }

    bool g_down(const GamepadState &s, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return s.current[i].btn & btn;
    }

    bool g_pressed(const GamepadState &s, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return !(s.previous[i].btn & btn) && (s.current[i].btn & btn);
    }

    bool g_released(const GamepadState &s, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return (s.previous[i].btn & btn) && !(s.current[i].btn & btn);
    }
}