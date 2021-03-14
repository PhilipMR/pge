#include "../include/edit_mesh.h"
#include <imgui/imgui.h>

namespace pge
{
    edit_MeshEditor::edit_MeshEditor(game_StaticMeshManager* sm)
        : m_meshManager(sm)
    {}

    void
    edit_MeshEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!m_meshManager->HasStaticMesh(entity)) {
            if (ImGui::Button("Add static mesh")) {
                m_meshManager->CreateStaticMesh(entity);
            } else {
                return;
            }
        }

        game_StaticMeshId mid = m_meshManager->GetStaticMeshId(entity);
        ImGui::Text("%s", m_meshManager->GetMesh(mid)->GetPath().c_str());
    }
} // namespace pge