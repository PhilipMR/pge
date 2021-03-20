#include "../include/input_keyboard.h"
#include "../include/input_events_win32.h"

namespace pge
{
    static const int NUM_KEYCODES = 512;
    static bool      s_keyDown[NUM_KEYCODES];
    static bool      s_keyPressed[NUM_KEYCODES];
    static bool      s_keyReleased[NUM_KEYCODES];

    void
    input_Win32KeyboardEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(hwnd);
        switch (uMsg) {
            case WM_KEYDOWN:
                if (!s_keyDown[wParam]) {
                    s_keyPressed[wParam] = true;
                }
                s_keyDown[wParam] = true;
                break;

            case WM_KEYUP:
                if (s_keyDown[wParam]) {
                    s_keyReleased[wParam] = true;
                }
                s_keyDown[wParam] = false;
                break;
        }
    }

    void
    input_KeyboardClearState()
    {
        for (int i = 0; i < NUM_KEYCODES; i++) {
            s_keyDown[i]     = false;
            s_keyPressed[i]  = false;
            s_keyReleased[i] = false;
        }
    }

    void
    input_KeyboardClearDelta()
    {
        for (int i = 0; i < NUM_KEYCODES; i++) {
            s_keyPressed[i]  = false;
            s_keyReleased[i] = false;
        }
    }

    bool
    input_KeyboardDown(input_KeyboardKey key)
    {
        return s_keyDown[static_cast<size_t>(key)];
    }

    bool
    input_KeyboardPressed(input_KeyboardKey key)
    {
        return s_keyPressed[static_cast<size_t>(key)];
    }

    bool
    input_KeyboardReleased(input_KeyboardKey key)
    {
        return s_keyReleased[static_cast<size_t>(key)];
    }
} // namespace pge
