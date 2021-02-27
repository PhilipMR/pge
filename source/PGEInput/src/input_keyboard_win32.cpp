#include "../include/input_keyboard.h"
#include "../include/input_events_win32.h"

namespace pge
{
    static const int kWin32KeyCodes = 512;
    static bool s_key_down[kWin32KeyCodes];
    static bool s_key_pressed[kWin32KeyCodes];
    static bool s_key_released[kWin32KeyCodes];

    void input_Win32KeyboardEvents(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        UNREFERENCED_PARAMETER(hwnd);
        switch (uMsg)
        {
            case WM_KEYDOWN:
                if (!s_key_down[wParam]) {
                    s_key_pressed[wParam] = true;
                }
                s_key_down[wParam] = true;
                break;
            case WM_KEYUP:
                if (s_key_down[wParam]) {
                    s_key_released[wParam] = true;
                }
                s_key_down[wParam] = false;
                break;
        }
    }

    void input_KeyboardClearState()
    {
        for (int i = 0; i < kWin32KeyCodes; i++) {
            //s_key_down[i] = false;
            s_key_pressed[i] = false;
            s_key_released[i] = false;
        }
    }

    void input_KeyboardClearDelta()
    {
        for (int i = 0; i < kWin32KeyCodes; i++) {
            s_key_pressed[i] = false;
            s_key_released[i] = false;
        }
    }

    bool input_KeyboardDown(input_KeyboardKey key)
    {
        return s_key_down[static_cast<size_t>(key)];
    }

    bool input_KeyboardPressed(input_KeyboardKey key)
    {
        return s_key_pressed[static_cast<size_t>(key)];
    }

    bool input_KeyboardReleased(input_KeyboardKey key)
    {
        return s_key_released[static_cast<size_t>(key)];
    }
}


