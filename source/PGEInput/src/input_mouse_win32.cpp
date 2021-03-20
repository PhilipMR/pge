#include "../include/input_mouse.h"
#include "../include/input_events_win32.h"
#include <math_vec2.h>

namespace pge
{
    static const size_t NUM_BUTTONS = static_cast<size_t>(input_MouseButton::NUM_BUTTONS);
    static math_Vec2    s_mousePosition;
    static math_Vec2    s_mouseDelta;
    static bool         s_mouseButtonDown[NUM_BUTTONS];
    static bool         s_mouseButtonPressed[NUM_BUTTONS];
    static bool         s_mouseButtonReleased[NUM_BUTTONS];

    void
    input_Win32MouseEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(hwnd);
        switch (uMsg) {
            case WM_MOUSEMOVE: {
                math_Vec2 position = math_Vec2((float)(lParam & 0x0000FFFF), (float)((lParam & 0xFFFF0000) >> 16));
                if (math_LengthSquared(s_mousePosition) > 0) {
                    s_mouseDelta = position - s_mousePosition;
                }
                s_mousePosition = position;
            } break;

            case WM_LBUTTONDOWN:
                if (!s_mouseButtonDown[static_cast<size_t>(input_MouseButton::LEFT)]) {
                    s_mouseButtonPressed[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                break;

            case WM_LBUTTONUP:
                if (s_mouseButtonDown[static_cast<size_t>(input_MouseButton::LEFT)]) {
                    s_mouseButtonReleased[static_cast<size_t>(input_MouseButton::LEFT)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::LEFT)] = false;
                break;

            case WM_MBUTTONDOWN:
                if (!s_mouseButtonDown[static_cast<size_t>(input_MouseButton::MIDDLE)]) {
                    s_mouseButtonPressed[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                break;

            case WM_MBUTTONUP:
                if (s_mouseButtonDown[static_cast<size_t>(input_MouseButton::MIDDLE)]) {
                    s_mouseButtonReleased[static_cast<size_t>(input_MouseButton::MIDDLE)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::MIDDLE)] = false;
                break;

            case WM_RBUTTONDOWN:
                if (!s_mouseButtonDown[static_cast<size_t>(input_MouseButton::RIGHT)]) {
                    s_mouseButtonPressed[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                break;

            case WM_RBUTTONUP:
                if (s_mouseButtonDown[static_cast<size_t>(input_MouseButton::RIGHT)]) {
                    s_mouseButtonReleased[static_cast<size_t>(input_MouseButton::RIGHT)] = true;
                }
                s_mouseButtonDown[static_cast<size_t>(input_MouseButton::RIGHT)] = false;
                break;
        }
    }

    void
    input_MouseClearState()
    {
        s_mouseDelta = math_Vec2(0, 0);
        for (int i = 0; i < NUM_BUTTONS; i++) {
            s_mouseButtonDown[i]     = false;
            s_mouseButtonPressed[i]  = false;
            s_mouseButtonReleased[i] = false;
        }
    }

    void
    input_MouseClearDelta()
    {
        s_mouseDelta = math_Vec2(0, 0);
        for (int i = 0; i < NUM_BUTTONS; i++) {
            s_mouseButtonPressed[i]  = false;
            s_mouseButtonReleased[i] = false;
        }
    }

    bool
    input_MouseButtonDown(input_MouseButton button)
    {
        return s_mouseButtonDown[static_cast<size_t>(button)];
    }

    bool
    input_MouseButtonPressed(input_MouseButton button)
    {
        return s_mouseButtonPressed[static_cast<size_t>(button)];
    }

    bool
    input_MouseButtonReleased(input_MouseButton button)
    {
        return s_mouseButtonReleased[static_cast<size_t>(button)];
    }

    math_Vec2
    input_MousePosition()
    {
        return s_mousePosition;
    }

    math_Vec2
    input_MouseDelta()
    {
        return s_mouseDelta;
    }
} // namespace pge