#ifndef PGE_OS_OS_DISPLAY_WIN32_H
#define PGE_OS_OS_DISPLAY_WIN32_H

#include "os_display.h"
#include <Windows.h>

namespace pge
{
    class os_DisplayWin32 : public os_Display {
    public:
        os_DisplayWin32(const char* title, int width, int height, WNDPROC wndProc);
        HWND GetWindowHandle();
    };
} // namespace pge

#endif