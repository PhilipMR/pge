#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include "edit_transform.h"
#include "edit_entity.h"
#include "edit_camera.h"
#include "edit_light.h"
#include "edit_game_view.h"

#include <gfx_render_target.h>
#include <math_vec2.h>
#include <game_light.h>
#include <game_world.h>

#include <imgui/imgui.h>
#include <memory>
#include <vector>

namespace pge
{
    class core_Display;
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;
    class gfx_RenderTarget;
    class res_ResourceManager;
    class game_World;

    void edit_Initialize(core_Display* display, gfx_GraphicsAdapter* graphics);
    void edit_Shutdown();

    class game_World;


    class edit_Editor {
        struct {
            ImTextureID          worldNode;
            ImTextureID          worldNodeSelected;
            ImTextureID          playButton;
            ImTextureID          pauseButton;
            const gfx_Texture2D* pointLight;
            const gfx_Texture2D* camera;
        } m_icons;

        enum
        {
            EDITOR_MODE_EDIT,
            EDITOR_MODE_PLAY
        } m_editMode;

        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_GraphicsDevice*  m_graphicsDevice;
        res_ResourceManager* m_resources;

        std::unique_ptr<game_World>                        m_world;
        edit_TransformGizmo                                m_transformGizmo;
        std::vector<std::unique_ptr<edit_ComponentEditor>> m_componentEditors;

        game_Entity       m_selectedEntity;
        bool              m_drawGrid;
        bool              m_drawGizmos;
        math_Vec2         m_gameWindowPos;
        math_Vec2         m_gameWindowSize;
        edit_CommandStack m_commandStack;

        edit_TransformEditor* m_transformEditor;
        edit_LightEditor      m_lightEditor;
        edit_CameraEditor     m_cameraEditor;

        edit_GameView m_gameView;

        game_Entity m_editCamera;

        constexpr static const math_Vec2 PREVIEW_RESOLUTION = math_Vec2(600.f, 600.f);
        gfx_RenderTarget                 m_previewRT;

    public:
        edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);

        void        LoadWorld(const char* path);
        game_World& GetWorld();
        bool        UpdateAndDraw();

    private:
        game_Entity EntityAtCursor() const;
        void        DrawGizmos();

        void        HandleEvents();
        void        DrawMenuBar();
        bool        DrawGameView();
        void        DrawEntityTree();
        void        DrawInspector();
        void        DrawLog();
        void        DrawResources();
        ImTextureID RenderMeshPreviewTexture(const res_Mesh* mesh, const res_Material* material);
    };
} // namespace pge

#endif