#include "../include/os_display_win32.h"
#include <diag_assert.h>

namespace pge
{
    struct os_Display::os_DisplayImpl {
        HWND m_hwnd;
        int  m_width;
        int  m_height;
        bool m_closeRequested;
    };

    os_Display::os_Display()
        : m_impl(new os_DisplayImpl)
    {}

    os_Display::~os_Display() = default;

    os_DisplayWin32::os_DisplayWin32(const char* title, int width, int height, WNDPROC wndProc)
    {
        m_impl->m_width          = width;
        m_impl->m_height         = height;
        m_impl->m_closeRequested = false;

        HINSTANCE hInstance    = GetModuleHandle(nullptr);
        WNDCLASS  wndClass     = {};
        wndClass.style         = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc   = wndProc;
        wndClass.hInstance     = hInstance;
        wndClass.hbrBackground = (HBRUSH)COLOR_ACTIVEBORDER;
        wndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wndClass.lpszClassName = "GameWindowClass";
        diag_VerifyWithReason(RegisterClass(&wndClass), "Could not register the game window class.");

        const DWORD wstyle = (WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME)) | WS_VISIBLE;
        RECT        wrect  = {0};
        wrect.right        = width;
        wrect.bottom       = height;
        AdjustWindowRect(&wrect, wstyle, false);
        m_impl->m_hwnd = CreateWindow(wndClass.lpszClassName,
                                      title,
                                      wstyle,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      wrect.right - wrect.left,
                                      wrect.bottom - wrect.top,
                                      nullptr,
                                      nullptr,
                                      hInstance,
                                      nullptr);
        diag_AssertWithReason(m_impl->m_hwnd != nullptr, "Could not create the game window");
    }

    HWND
    os_DisplayWin32::GetWindowHandle()
    {
        return m_impl->m_hwnd;
    }

    void
    os_Display::HandleEvents()
    {
        MSG msg = {0};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                m_impl->m_closeRequested = true;
        }
    }

    int
    os_Display::GetWidth() const
    {
        return m_impl->m_width;
    }

    int
    os_Display::GetHeight() const
    {
        return m_impl->m_height;
    }

    bool
    os_Display::IsCloseRequested() const
    {
        return m_impl->m_closeRequested;
    }

} // namespace pge