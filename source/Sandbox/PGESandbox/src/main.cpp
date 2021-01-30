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
#include <math_vec4.h>
#include <math_mat4x4.h>
#include <math_quat.h>
#include <res_resource_manager.h>
#include <game_scene.h>
#include <game_camera.h>
#include <game_transform.h>

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
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

        // Load resources
        res_ResourceManager resources(&graphicsAdapter);
        const res_Mesh*     mesh     = resources.GetMesh(R"(C:\Users\phili\Desktop\suzanne\meshes\Suzanne.001.mesh)");
        const res_Material* material = resources.GetMaterial(R"(C:\Users\phili\Desktop\suzanne.mat)");

        game_Scene scene;


        game_Camera camera(60.0f, 1280.0f / 720.0f, 0.01f, 100.0f);
        camera.GetTransform()->SetPosition(math_Vec3(0, 0, 5));

        game_Transform transform;

        // Create transforms constant buffer
        struct CBTransforms {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
        } transforms;
        transforms.modelMatrix = transform.GetModelMatrix();
        transforms.viewMatrix  = camera.GetViewMatrix();
        transforms.projMatrix  = camera.GetProjectionMatrix();
        gfx_ConstantBuffer transformCBuffer(&graphicsAdapter, &transforms, sizeof(CBTransforms), gfx_BufferUsage::DYNAMIC);

        // Rotate animation lambda
        auto rotateClockwise = [&](float speed) {
            transform.Rotate(math_Vec3(0, 1, 0), -speed);
            transforms.modelMatrix = transform.GetModelMatrix();
            transformCBuffer.Update(&transforms, sizeof(CBTransforms));
        };

        // Bind the transform buffer, the mesh and the active material
        transformCBuffer.BindVS(0);
        mesh->Bind();
        material->Bind();

        // Create and use a graphics device for in the draw loop
        gfx_GraphicsDevice graphicsDevice(&graphicsAdapter);
        while (!display.IsCloseRequested()) {
            display.HandleEvents();
            graphicsDevice.Clear();
            rotateClockwise(360.0f / 60.0f * 0.25f);
            graphicsDevice.DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
            graphicsDevice.Present();
        }
    }

    return 0;
}