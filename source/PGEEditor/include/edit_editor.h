#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include "edit_editor_views.h"

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
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_GraphicsDevice*  m_graphicsDevice;
        res_ResourceManager* m_resources;

        std::unique_ptr<game_World> m_world;

        // Editor state
        game_Entity       m_selectedEntity = game_EntityId_Invalid;
        edit_CommandStack m_commandStack;

        // Editor views
        edit_LogView             m_logView;
        edit_EditView            m_editView;
        edit_EntityHierarchyView m_hierarchyView;
        edit_ResourceView        m_resourceView;
        edit_InspectorView       m_inspectorView;

    public:
        edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);
        bool UpdateAndDraw();

    private:
        void HandleShortcuts();
    };
} // namespace pge

#endif