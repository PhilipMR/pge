#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include "edit_transform.h"
#include "edit_entity.h"
#include <math_vec2.h>
#include <game_light.h>
#include <game_scene.h>
#include <memory>
#include <vector>
#include <imgui/imgui.h>

namespace pge
{
    class os_Display;
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;
    class gfx_RenderTarget;
    class res_ResourceManager;
    class game_Scene;

    void edit_Initialize(os_Display* display, gfx_GraphicsAdapter* graphics);
    void edit_Shutdown();

    class game_Scene;

    enum class edit_EditMode
    {
        NONE,
        TRANSLATE,
        SCALE,
        ROTATE
    };


    class edit_Editor {
        struct {
            ImTextureID          sceneNode;
            ImTextureID          sceneNodeSelected;
            ImTextureID          playButton;
            ImTextureID          pauseButton;
            const gfx_Texture2D* pointLight;
        } m_icons;

        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_GraphicsDevice*  m_graphicsDevice;
        res_ResourceManager* m_resources;

        std::unique_ptr<game_Scene>                        m_scene;
        edit_TranslateTool                                 m_translator;
        edit_ScalingTool                                   m_scaler;
        edit_RotationTool                                  m_rotator;
        std::vector<std::unique_ptr<edit_ComponentEditor>> m_componentEditors;

        edit_EditMode     m_editMode;
        game_Entity       m_selectedEntity;
        math_Vec2         m_gameWindowPos;
        math_Vec2         m_gameWindowSize;
        edit_CommandStack m_commandStack;

    public:
        edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);

        void        LoadScene(const char* path);
        game_Scene& GetScene();
        bool        UpdateAndDraw(const gfx_RenderTarget* target);

    private:
        game_Entity SelectEntity() const;
        void        DrawGizmos() const;

        void HandleEvents();
        void DrawMenuBar();
        bool DrawGameView(const gfx_RenderTarget* target);
        void DrawEntityTree();
        void DrawInspector();
        void DrawExplorer();
    };
} // namespace pge

#endif