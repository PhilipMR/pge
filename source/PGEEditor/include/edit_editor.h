#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include <game_entity.h>

namespace pge
{
    class os_Display;
    class gfx_GraphicsAdapter;
    class gfx_RenderTarget;
    class game_Scene;

    void edit_Initialize(os_Display* display, gfx_GraphicsAdapter* graphics);
    void edit_Shutdown();
    void edit_BeginFrame();
    void edit_EndFrame();

    class game_Scene;
    class edit_Editor {
        game_EntityId m_selectedEntity;

    public:
        edit_Editor();
        void HandleEvents(const game_Scene* scene);
        void DrawMenuBar();
        bool DrawRenderTarget(const char* title,const gfx_RenderTarget* target);
        void DrawEntityTree(const game_Scene* scene);
        void DrawInspector(const game_Scene* scene);
        void DrawExplorer();
    };
} // namespace pge

#endif