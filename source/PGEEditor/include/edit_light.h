#ifndef PGE_EDITOR_EDIT_LIGHT_H
#define PGE_EDITOR_EDIT_LIGHT_H

#include "edit_component.h"
#include <game_light.h>

namespace pge
{
    class edit_LightEditor : public edit_ComponentEditor {
        game_LightManager* m_lightManager;

    public:
        edit_LightEditor(game_LightManager* lm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };
}

#endif