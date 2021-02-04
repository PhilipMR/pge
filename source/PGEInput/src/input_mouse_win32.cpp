#include "../include/input_mouse.h"
#include "../include/input_events_win32.h"
#include <math_vec2.h>

namespace pge
{
    static const size_t NumButtons = static_cast<size_t>(input_MouseButton::NUM_BUTTONS);
    static math_Vec2 s_mouse_position;
    static math_Vec2 s_mouse_delta;
    static bool      s_mouse_button_down[NumButtons];
    static bool      s_mouse_button_pressed[NumButtons];
    static bool      s_mouse_button_released[NumButtons];

    void
    input_Win32MouseEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(hwnd);
        switch (uMsg) {
            case WM_MOUSEMOVE: {
                math_Vec2 position = math_Vec2((float)(lParam & 0x0000FFFF), (float)((lParam & 0xFFFF0000) >> 16));
                if (math_LengthSquared(s_mouse_position) > 0) {
                    s_mouse_delta = position - s_mouse_position;
                }
                s_mouse_position = position;
            } break;

            case WM_LBUTTONDOWN:
                if (!s_mouse_button_down[static_cast<size_t>(input_MouseButton::LEFT)]) {
                    s_mouse_button_pressed[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                break;

            case WM_LBUTTONUP:
                if (s_mouse_button_down[static_cast<size_t>(input_MouseButton::LEFT)]) {
                    s_mouse_button_released[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::LEFT)] = false;
                break;

            case WM_MBUTTONDOWN:
                if (!s_mouse_button_down[static_cast<size_t>(input_MouseButton::MIDDLE)]) {
                    s_mouse_button_pressed[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                break;

            case WM_MBUTTONUP:
                if (s_mouse_button_down[static_cast<size_t>(input_MouseButton::MIDDLE)]) {
                    s_mouse_button_released[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::MIDDLE)] = false;
                break;

            case WM_RBUTTONDOWN:
                if (!s_mouse_button_down[static_cast<size_t>(input_MouseButton::RIGHT)]) {
                    s_mouse_button_pressed[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                break;

            case WM_RBUTTONUP:
                if (s_mouse_button_down[static_cast<size_t>(input_MouseButton::RIGHT)]) {
                    s_mouse_button_released[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                }
                s_mouse_button_down[static_cast<size_t>(input_MouseButton::RIGHT)] = false;
                break;
        }
    }

    void
    input_MouseClearDelta()
    {
        s_mouse_delta = math_Vec2(0, 0);
        for (int i = 0; i < static_cast<size_t>(input_MouseButton::NUM_BUTTONS); i++) {
            s_mouse_button_pressed[i]  = false;
            s_mouse_button_released[i] = false;
        }
    }

    bool
    input_MouseButtonDown(input_MouseButton button)
    {
        return s_mouse_button_down[static_cast<size_t>(button)];
    }

    bool
    input_MouseButtonPressed(input_MouseButton button)
    {
        return s_mouse_button_pressed[static_cast<size_t>(button)];
    }

    bool
    input_MouseButtonReleased(input_MouseButton button)
    {
        return s_mouse_button_released[static_cast<size_t>(button)];
    }

    math_Vec2
    input_MousePosition()
    {
        return s_mouse_position;
    }

    math_Vec2
    input_MouseDelta()
    {
        return s_mouse_delta;
    }
} // namespace pge