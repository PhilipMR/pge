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
        edit_TranslateTool                                 m_translator;
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