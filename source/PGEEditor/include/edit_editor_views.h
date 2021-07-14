#ifndef PGE_EDITOR_EDIT_EDITOR_VIEWS_H
#define PGE_EDITOR_EDIT_EDITOR_VIEWS_H

#include "edit_command.h"
#include <game_world.h>

namespace pge
{
    static const char* PATH_TO_LAYOUT_INI = "layout.ini";

    void edit_DrawMainMenuBar(game_World* world, edit_CommandStack* cstack);
    void edit_DrawGameViewMenu(bool* isPlay, bool* drawGrid, bool* drawGizmos);
    void edit_DrawLogView();

    class edit_GameView {
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_RenderTarget     m_rtGameMs;
        gfx_RenderTarget     m_rtGame;
        const res_Effect*    m_multisampleEffect;

    public:
        edit_GameView(gfx_GraphicsAdapter* graphicsAdapter, res_ResourceManager* resources, unsigned width, unsigned height);
        void DrawOnGUI(game_World* world, const math_Vec2& size, const math_Mat4x4& view, const math_Mat4x4& proj);
    };

    class edit_EntityHierarchyView {
        game_Entity m_hoveredEntity;

        void ApplyDrop(game_World* world, const game_Entity& targetEntity);
        void DestroyRecursive(game_World* world, const game_Entity& entity, std::vector<game_Entity>* entitiesRemove);
        bool DrawEntityNode(game_World*               world,
                            const game_Entity&        entity,
                            game_Entity*              selectedEntity,
                            std::vector<game_Entity>* entitiesRemove,
                            bool                      isLeaf);
        void DrawLocalTree(game_World* world, const game_Entity& entity, game_Entity* selectedEntity, std::vector<game_Entity>* entitiesRemove);

    public:
        void DrawOnGUI(game_World* world, game_Entity* selectedEntity, edit_CommandStack* cstack);
    };

} // namespace pge

#endif