#ifndef PGE_EDITOR_EDIT_EDITOR_H
#define PGE_EDITOR_EDIT_EDITOR_H

namespace pge
{
    class os_Display;
    class gfx_GraphicsAdapter;
    class game_Scene;

    void edit_Gui_Initialize(os_Display* display, gfx_GraphicsAdapter* graphics);
    void edit_Gui_Shutdown();
    void edit_Gui_BeginFrame();
    void edit_Gui_EndFrame();
    bool edit_Gui_IsFocused();

    void edit_DrawLeftPanel();
    void edit_DrawRightPanel();
    void edit_DrawTopPanel();
    void edit_DrawBottomPanel();
} // namespace pge

#endif