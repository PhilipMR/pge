#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

#include <game_entity.h>
#include <math_vec2.h>
#include <math_vec3.h>

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

    enum class edit_EditMode
    {
        NONE,
        TRANSLATE,
    };

    enum class edit_Axis
    {
        NONE,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ
    };

    class edit_Editor {
        edit_EditMode m_editMode;
        edit_Axis     m_editAxis;
        math_Vec3     m_preTransformPosition;
        game_EntityId m_selectedEntity;
        math_Vec2     m_gameWindowPos;
        math_Vec2     m_gameWindowSize;

    public:
        edit_Editor();
        void HandleEvents(game_Scene* scene);
        void DrawMenuBar();
        bool DrawRenderTarget(const char* title, const gfx_RenderTarget* target);
        void DrawEntityTree(const game_Scene* scene);
        void DrawInspector(const game_Scene* scene);
        void DrawExplorer();
    };
} // namespace pge

#endif