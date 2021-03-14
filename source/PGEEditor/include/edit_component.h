#ifndef PGE_EDITOR_EDIT_COMPONENT_H
#define PGE_EDITOR_EDIT_COMPONENT_H

#include <game_entity.h>

namespace pge
{
    class edit_ComponentEditor {
    public:
        virtual ~edit_ComponentEditor()                       = default;
        virtual void UpdateAndDraw(const game_Entity& entity) = 0;
    };
}

#endif