#ifdef _DEBUG
#    define new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#    define malloc(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
#    define free(data)   _free_dbg(data, _NORMAL_BLOCK)
#    define _CRTDBG_MAP_ALLOC
#    include <stdlib.h>
#    include <crtdbg.h>
#endif

#include <os_display_win32.h>
#include <gfx_graphics_adapter_d3d11.h>
#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <res_resource_manager.h>
#include <game_scene.h>
#include <game_camera.h>
#include <game_transform.h>
#include <input_events_win32.h>

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    pge::input_Win32KeyboardEvents(hwnd, uMsg, wParam, lParam);
    pge::input_Win32MouseEvents(hwnd, uMsg, wParam, lParam);
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
        // Create graphics context
        os_DisplayWin32          display("PGE Sandbox", 1280, 720, WindowProc);
        gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());
        gfx_GraphicsDevice       graphicsDevice(&graphicsAdapter);

        // Load resources
        res_ResourceManager resources(&graphicsAdapter);
        const res_Mesh*     mesh     = resources.GetMesh(R"(data\meshes\suzanne\Suzanne.001.mesh)");
        const res_Material* material = resources.GetMaterial(R"(data\meshes\suzanne\Suzanne.001.mat)");

        game_Scene scene(&graphicsAdapter, &graphicsDevice);
        game_StaticMesh* entity1  = scene.CreateStaticMesh(mesh, material, game_Transform(math_Vec3(-3, 0, 0)));
        game_StaticMesh* entity2 = scene.CreateStaticMesh(mesh, material, game_Transform(math_Vec3(3, 0, 0)));

        // Create and use a graphics device for in the draw loop
        while (!display.IsCloseRequested()) {
            input_KeyboardClearDelta();
            input_MouseClearDelta();

            display.HandleEvents();
            graphicsDevice.Clear();

            const float rotSpeed = 360.0f / 60.0f * 0.1f;
            entity1->GetTransform()->Rotate(math_Vec3(0, 1, 0), -rotSpeed);
            entity2->GetTransform()->Rotate(math_Vec3(0, 1, 0), rotSpeed);
            scene.Draw();
            graphicsDevice.Present();
        }
    }

    return 0;
}