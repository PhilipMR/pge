#ifndef PGE_INPUT_INPUT_EVENTS_WIN32_H
#define PGE_INPUT_INPUT_EVENTS_WIN32_H

#include <Windows.h>

namespace pge
{
    void input_Win32KeyboardEvents(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    );

    void input_Win32MouseEvents(
        HWND   hwnd,
        UINT   uMsg,
        WPARAM wParam,
        LPARAM lParam
    );
}

#endif