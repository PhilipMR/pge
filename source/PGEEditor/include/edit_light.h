#ifndef PGE_EDITOR_EDIT_LIGHT_H
#define PGE_EDITOR_EDIT_LIGHT_H

#include <game_light.h>

namespace pge
{
    // NOT a edit_ComponentEditor because it shouldn't be available to non-light entities.
    class edit_LightEditor  {
        game_LightManager* m_lightManager;

    public:
        edit_LightEditor(game_LightManager* lm);
        void UpdateAndDraw(const game_Entity& entity);
    };
}

#endif