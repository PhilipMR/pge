#include <imgui/imgui.h>

#ifdef _DEBUG
#    define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#    define malloc(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#    define free(data)   _free_dbg(data, _NORMAL_BLOCK)
#    define _CRTDBG_MAP_ALLOC
#    include <stdlib.h>
#    include <crtdbg.h>
#endif

#include <core_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <gfx_graphics_device.h>
#include <gfx_debug_draw.h>
#include <res_resource_manager.h>
#include <edit_events_win32.h>
#include <edit_editor.h>
#include <input_keyboard.h>
#include <input_mouse.h>
#include <input_events_win32.h>

static bool                           s_hoveringGameWindow = false;
static pge::gfx_GraphicsAdapterD3D11* s_graphicsAdapter    = nullptr;
static pge::gfx_GraphicsDevice*       s_graphicsDevice     = nullptr;

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    if (uMsg == WM_SIZE && s_graphicsAdapter) {
        DWORD width  = LOWORD(lParam);
        DWORD height = HIWORD(lParam);
        if (width * height != 0) {
            s_graphicsAdapter->ResizeBackBuffer(LOWORD(lParam), HIWORD(lParam));
            s_graphicsDevice->SetViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        }
    }

    pge::edit_Win32Events(hwnd, uMsg, wParam, lParam);
    if (s_hoveringGameWindow) {
        pge::input_Win32KeyboardEvents(hwnd, uMsg, wParam, lParam);
        pge::input_Win32MouseEvents(hwnd, uMsg, wParam, lParam);
    } else {
        pge::input_KeyboardClearDelta();
        pge::input_MouseClearDelta();
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int
main()
{
    using namespace pge;

#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
#endif

    const math_Vec2 resolution(1920, 1080);

    core_DisplayWin32        display("PGE Sandbox", resolution.x, resolution.y, WindowProc);
    gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());
    gfx_GraphicsDevice       graphicsDevice(&graphicsAdapter);
    res_ResourceManager      resources(&graphicsAdapter);

    s_graphicsAdapter = &graphicsAdapter;
    s_graphicsDevice  = &graphicsDevice;
    ShowWindow(display.GetWindowHandle(), SW_SHOWMAXIMIZED);

    gfx_DebugDraw_Initialize(&graphicsAdapter, &graphicsDevice);
    edit_Initialize(&display, &graphicsAdapter);

    edit_Editor editor(&graphicsAdapter, &graphicsDevice, &resources);
    game_World& world = editor.GetWorld();
    while (!display.IsCloseRequested()) {
        input_KeyboardClearDelta();
        input_MouseClearDelta();
        display.HandleEvents();

        world.GarbageCollect();

        gfx_RenderTarget_BindMainRTV(&graphicsAdapter);
        gfx_RenderTarget_ClearMainRTV(&graphicsAdapter);
        bool isHovered = editor.UpdateAndDraw();
        if (s_hoveringGameWindow && !isHovered) {
            input_KeyboardClearState();
            input_MouseClearState();
        }
        s_hoveringGameWindow = isHovered;

        graphicsDevice.Present();
    }
    gfx_DebugDraw_Shutdown();
    edit_Shutdown();

    return 0;
}