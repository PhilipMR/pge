#ifndef PGE_EDITOR_EDIT_MESH_H
#define PGE_EDITOR_EDIT_MESH_H

#include "edit_component.h"
#include <core_fileutils.h>

namespace pge
{
    class game_StaticMeshManager;
    class res_ResourceManager;
    class edit_MeshEditor : public edit_ComponentEditor {
        game_StaticMeshManager*  m_meshManager;
        res_ResourceManager*     m_resources;
        time_t                   m_lastFileSync;
        std::vector<core_FSItem> m_meshItems;
        std::vector<core_FSItem> m_matItems;

    public:
        edit_MeshEditor(game_StaticMeshManager* sm, res_ResourceManager* resources);
        virtual void UpdateAndDraw(const game_Entity& entity) override;

    private:
        void SyncResourcePaths();
    };
} // namespace pge

#endif