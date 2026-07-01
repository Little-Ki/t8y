#include "input/gamepad.h"

#include <algorithm>

namespace t8::input
{

    void gamepad_flush(GamepadState &state)
    {
        state.previous[0] = state.current[0];
        state.previous[1] = state.current[1];
        state.previous[2] = state.current[2];
        state.previous[3] = state.current[3];
    }

    void gamepad_join(GamepadState &state, uint32_t id)
    {
        for (auto &i : state.mapper)
        {
            if (i == 0)
            {
                i = id;
                return;
            }
        }
    }

    void gamepad_remove(GamepadState &state, uint32_t id)
    {
        for (auto &i : state.mapper)
        {
            if (i == id)
            {
                i = 0;
                break;
            }
        }

        for (auto i = 0; i < 4; i++)
        {
            if (state.mapper[i] != 0)
                continue;

            for (auto j = i + 1; j < 4; j++)
            {
                if (state.mapper[j] == 0)
                    continue;
                std::swap(state.mapper[i], state.mapper[j]);
                return;
            }
        }
    }

    void gamepad_button(GamepadState &state, uint32_t id, uint8_t btn, bool down)
    {
        for (auto i = 0; i < 4; i++)
        {
            if (state.mapper[i] == id)
            {
                down ? state.current[i].btn |= btn : state.current[i].btn &= ~btn;
                return;
            }
        }
    }

    bool gamepad_down(const GamepadState &state, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return state.current[i].btn & btn;
    }

    bool gamepad_pressed(const GamepadState &state, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return !(state.previous[i].btn & btn) && (state.current[i].btn & btn);
    }

    bool gamepad_released(const GamepadState &state, uint8_t i, uint8_t btn)
    {
        if (i > 3)
            return false;
        return (state.previous[i].btn & btn) && !(state.current[i].btn & btn);
    }
}