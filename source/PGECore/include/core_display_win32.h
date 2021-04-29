#ifndef PGE_CORE_CORE_DISPLAY_WIN32_H
#define PGE_CORE_CORE_DISPLAY_WIN32_H

#include "core_display.h"
#include <Windows.h>

namespace pge
{
    class core_DisplayWin32 : public core_Display {
    public:
        core_DisplayWin32(const char* title, int width, int height, WNDPROC wndProc);
        HWND GetWindowHandle();
    };
} // namespace pge

#endif