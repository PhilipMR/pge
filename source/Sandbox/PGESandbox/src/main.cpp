#include <imgui/imgui.h>

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
#include <gfx_render_target.h>
#include <res_resource_manager.h>
#include <game_scene.h>
#include <input_events_win32.h>
#include <edit_events_win32.h>
#include <edit_editor.h>
#include <gfx_debug_draw.h>

static bool s_hoveringGameWindow = false;

static LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
        PostQuitMessage(0);
    pge::edit_Win32Events(hwnd, uMsg, wParam, lParam);
    if (s_hoveringGameWindow) {
        pge::input_Win32KeyboardEvents(hwnd, uMsg, wParam, lParam);
        pge::input_Win32MouseEvents(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


pge::game_Entity       entities[3];
pge::game_TransformId  transforms[3];
pge::game_StaticMeshId meshes[3];

static void
InitializeScene(pge::game_Scene* scene, pge::res_ResourceManager* resources)
{
    using namespace pge;

    auto em = scene->GetEntityManager();
    auto mm = scene->GetEntityMetaDataManager();
    auto tm = scene->GetTransformManager();
    auto sm = scene->GetStaticMeshManager();

    const res_Mesh*     mesh     = resources->GetMesh(R"(data\meshes\cube\Cube.001.mesh)");
    const res_Material* material = resources->GetMaterial(R"(data\materials\checkers.mat)");

    em->CreateEntities(entities, 3);
    tm->CreateTransforms(entities, 3, transforms);
    sm->CreateStaticMeshes(entities, 3, meshes);

    for (size_t i = 0; i < 3; ++i) {
        const char* name = i == 0 ? "Alpha" : (i == 1 ? "Beta" : (i == 2 ? "Charlie" : "Unknown"));
        mm->CreateMetaData(entities[i], game_EntityMetaData(entities[i], name));
        sm->SetMesh(meshes[i], mesh);
        sm->SetMaterial(meshes[i], material);
    }

    tm->SetLocal(transforms[0], math_CreateTranslationMatrix(math_Vec3(-6, 5, 0)));
    tm->SetLocal(transforms[1], math_CreateTranslationMatrix(math_Vec3(6, 5, 0)));
    tm->SetLocal(transforms[2], math_CreateScaleMatrix(math_Vec3(10, 1, 10)));

    scene->GetCamera()->SetLookAt(math_Vec3(0, 10.0f, -10.0f), math_Vec3::Zero());
}

static void
UpdateScene(pge::game_Scene* scene)
{
    using namespace pge;
    auto        tm       = scene->GetTransformManager();
    const float rotSpeed = 360.0f / 60.0f * 0.1f;
    tm->Rotate(transforms[0], math_Vec3(0, 1, 0), -rotSpeed);
    tm->Rotate(transforms[1], math_Vec3(0, 1, 0), rotSpeed);

    scene->Update();
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
        const math_Vec2 resolution(1600, 900);

        // Create graphics context
        os_DisplayWin32          display("PGE Sandbox", resolution.x, resolution.y, WindowProc);
        gfx_GraphicsAdapterD3D11 graphicsAdapter(display.GetWindowHandle(), display.GetWidth(), display.GetHeight());
        gfx_GraphicsDevice       graphicsDevice(&graphicsAdapter);

        // Setup scene
        res_ResourceManager resources(&graphicsAdapter);
        game_Scene          scene(&graphicsAdapter, &graphicsDevice);
        InitializeScene(&scene, &resources);

        // Set up editor
        const float      resScale = 5.0f;
        gfx_RenderTarget rtGame(&graphicsAdapter, resolution.x * resScale, resolution.y * resScale, true, true);
        gfx_RenderTarget rtGameMs(&graphicsAdapter, resolution.x, resolution.y, false, false);
        edit_Initialize(&display, &graphicsAdapter);
        edit_Editor editor;

        gfx_DebugDraw_Initialize(&graphicsAdapter, &graphicsDevice);


        const res_Effect* screenTexEffect = resources.GetEffect("data/effects/screentex.effect");

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
            UpdateScene(&scene);

            // Draw scene to texture
            gfx_Texture2D_Unbind(&graphicsAdapter, 0);
            rtGame.Bind();
            rtGame.Clear();
            scene.Draw();

            gfx_DebugDraw_SetView(scene.GetCamera()->GetViewMatrix());
            gfx_DebugDraw_SetProjection(scene.GetCamera()->GetProjectionMatrix());

            const float thickness = 0.1f;
            const float length    = 100.0f;
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(length, 0, 0), math_Vec3(1, 0, 0), thickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, length, 0), math_Vec3(0, 1, 0), thickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, 0, length), math_Vec3(0, 0, 1), thickness);

            gfx_DebugDraw_Box(math_Vec3(10, 10, 10), math_Vec3(50, 50, 50), math_Vec3(1, 1, 1), thickness);
            gfx_DebugDraw_Point(math_Vec3(5, 5, 5), math_Vec3(1, 0, 1), thickness);
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

            edit_BeginFrame();
            editor.HandleEvents();
            editor.DrawMenuBar();
            s_hoveringGameWindow = editor.DrawRenderTarget("Game", &rtGameMs);
            editor.DrawEntityTree(&scene);
            editor.DrawInspector(&scene);
            editor.DrawExplorer();
            edit_EndFrame();

            graphicsDevice.Present();
        }
        gfx_DebugDraw_Shutdown();
        edit_Shutdown();
    }

    return 0;
}