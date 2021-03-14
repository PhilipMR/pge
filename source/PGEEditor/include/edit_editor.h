#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include <memory>
#include "edit_command.h"
#include "edit_transform.h"
#include <math_vec2.h>

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
    void edit_BeginFrame();
    void edit_EndFrame();

    class game_Scene;

    enum class edit_EditMode
    {
        NONE,
        TRANSLATE,
    };

    class edit_Editor {
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_GraphicsDevice*  m_graphicsDevice;
        res_ResourceManager* m_resources;

        std::unique_ptr<game_Scene>                        m_scene;
        std::vector<std::unique_ptr<edit_ComponentEditor>> m_componentEditors;

        edit_EditMode m_editMode;
        edit_Axis     m_editAxis;
        math_Vec3     m_preTransformPosition;
        game_Entity   m_selectedEntity;
        math_Vec2     m_gameWindowPos;
        math_Vec2     m_gameWindowSize;

        edit_CommandStack m_commandStack;

    public:
        edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);

        void LoadScene(const char* path);
        bool UpdateAndDraw(const gfx_RenderTarget* target);

    private:
        void HandleEvents(game_Scene* scene);
        void DrawMenuBar(game_Scene* scene);
        bool DrawGameView(game_Scene* scene, const gfx_RenderTarget* target, res_ResourceManager* resources);
        void DrawEntityTree(game_Scene* scene);
        void DrawInspector(game_Scene* scene, res_ResourceManager* resources);
        void DrawExplorer();
    };
} // namespace pge

#endif