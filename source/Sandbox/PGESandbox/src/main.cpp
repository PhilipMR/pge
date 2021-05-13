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
#include <gfx_render_target.h>
#include <res_resource_manager.h>
#include <game_world.h>
#include <input_events_win32.h>
#include <edit_events_win32.h>
#include <edit_editor.h>
#include <gfx_debug_draw.h>
#include <game_script.h>
#include <input_keyboard.h>
#include <input_mouse.h>

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

    // Scope main body so everything is deallocated by the end of main.
    {
        const math_Vec2 resolution(1920, 1080);

        // Create graphics context
        core_DisplayWin32        display("PGE Sandbox", resolution.x, resolution.y, WindowProc);
        gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());
        gfx_GraphicsDevice       graphicsDevice(&graphicsAdapter);
        s_graphicsAdapter = &graphicsAdapter;
        s_graphicsDevice  = &graphicsDevice;
        ShowWindow(display.GetWindowHandle(), SW_SHOWMAXIMIZED);

        // Setup scene
        res_ResourceManager resources(&graphicsAdapter);

        // Set up editor
        gfx_RenderTarget rtGame(&graphicsAdapter, resolution.x, resolution.y, true, true);
        gfx_RenderTarget rtGameMs(&graphicsAdapter, resolution.x, resolution.y, false, false);
        edit_Initialize(&display, &graphicsAdapter);
        edit_Editor editor(&graphicsAdapter, &graphicsDevice, &resources);
        editor.LoadWorld("test.world");
        game_World& world = editor.GetWorld();
        world.GetCamera()->SetLookAt(math_Vec3(10, 10, 10), math_Vec3(0, 0, 0));
        gfx_DebugDraw_Initialize(&graphicsAdapter, &graphicsDevice);


        const res_Effect*         screenTexEffect     = resources.GetEffect("data/effects/screentex.effect");
        const gfx_VertexAttribute screenTexAttribs[]  = {gfx_VertexAttribute("POSITION", gfx_VertexAttributeType::FLOAT2),
                                                        gfx_VertexAttribute("TEXTURECOORD", gfx_VertexAttributeType::FLOAT2)};
        const math_Vec2           screenTexVertices[] = {math_Vec2(-1, 1),
                                               math_Vec2(0, 0),
                                               math_Vec2(-1, -1),
                                               math_Vec2(0, 1),
                                               math_Vec2(1, -1),
                                               math_Vec2(1, 1),
                                               math_Vec2(1, 1),
                                               math_Vec2(1, 0)};
        const unsigned            screenTexIndices[]  = {0, 1, 2, 2, 3, 0};
        const res_Mesh            screenTexMesh(&graphicsAdapter,
                                     screenTexAttribs,
                                     sizeof(screenTexAttribs) / sizeof(gfx_VertexAttribute),
                                     screenTexVertices,
                                     sizeof(screenTexVertices),
                                     screenTexIndices,
                                     sizeof(screenTexIndices) / sizeof(unsigned));


        while (!display.IsCloseRequested()) {
            // Update input, window and scene
            input_KeyboardClearDelta();
            input_MouseClearDelta();
            display.HandleEvents();
            world.Update();

            // Draw scene to texture
            gfx_Texture2D_Unbind(&graphicsAdapter, 0);
            rtGame.Bind();
            rtGame.Clear();
            world.Draw();

            gfx_DebugDraw_SetView(world.GetCamera()->GetViewMatrix());
            gfx_DebugDraw_SetProjection(world.GetCamera()->GetProjectionMatrix());


            gfx_DebugDraw_Flush();

            // Redraw screen to intermediate texture (additional multisampling)
            gfx_Texture2D_Unbind(&graphicsAdapter, 0);
            rtGameMs.Bind();
            rtGameMs.Clear();
            screenTexMesh.Bind();
            screenTexEffect->VertexShader()->Bind();
            screenTexEffect->PixelShader()->Bind();
            rtGame.BindTexture(0);
            graphicsDevice.DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, 6);

            // Draw editor (with scene texture to window) to main render target
            gfx_RenderTarget_BindMainRTV(&graphicsAdapter);
            gfx_RenderTarget_ClearMainRTV(&graphicsAdapter);

            bool isHovered = editor.UpdateAndDraw(&rtGameMs);
            if (s_hoveringGameWindow && !isHovered) {
                input_KeyboardClearState();
                input_MouseClearState();
            }
            s_hoveringGameWindow = isHovered;


            graphicsDevice.Present();
        }
        gfx_DebugDraw_Shutdown();
        edit_Shutdown();
    }

    return 0;
}