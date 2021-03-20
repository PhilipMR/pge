#ifndef PGE_EDITOR_EDIT_MESH_H
#define PGE_EDITOR_EDIT_MESH_H

#include "edit_component.h"
#include <game_static_mesh.h>
#include <res_resource_manager.h>
#include <time.h>
#include <os_file.h>

namespace pge
{
    class edit_MeshEditor : public edit_ComponentEditor {
        game_StaticMeshManager*  m_meshManager;
        res_ResourceManager*     m_resources;
        time_t                   m_lastFileSync;
        std::vector<os_ListItem> m_meshItems;
        std::vector<os_ListItem> m_matItems;

    public:
        edit_MeshEditor(game_StaticMeshManager* sm, res_ResourceManager* resources);
        virtual void UpdateAndDraw(const game_Entity& entity) override;

    private:
        void SyncResourcePaths();
    };
} // namespace pge

#endif