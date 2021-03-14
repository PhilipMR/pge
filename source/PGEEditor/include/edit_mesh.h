#ifndef PGE_EDITOR_EDIT_MESH_H
#define PGE_EDITOR_EDIT_MESH_H

#include "edit_component.h"
#include <game_static_mesh.h>

namespace pge
{
    class game_StaticMeshManager;
    class edit_MeshEditor : public edit_ComponentEditor {
        game_StaticMeshManager* m_meshManager;

    public:
        edit_MeshEditor(game_StaticMeshManager* sm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };
}

#endif