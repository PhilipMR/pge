#include "../include/edit_mesh.h"
#include <imgui/imgui.h>

namespace pge
{
    edit_MeshEditor::edit_MeshEditor(game_StaticMeshManager* sm, res_ResourceManager* resources)
        : m_meshManager(sm)
        , m_resources(resources)
        , m_lastFileSync()
    {}


    void
    edit_MeshEditor::SyncResourcePaths()
    {
        static const unsigned SECONDS_PER_SYNC = 10;
        time_t                tm;
        time(&tm);
        double diffSecs = difftime(tm, m_lastFileSync);
        if (diffSecs < SECONDS_PER_SYNC)
            return;
        else
            m_lastFileSync = tm;

        m_meshItems = os_ListItemsWithExtension("data", "mesh", true);
        m_matItems  = os_ListItemsWithExtension("data", "mat", true);
    }

    void
    edit_MeshEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Static mesh"))
            return;

        if (!m_meshManager->HasStaticMesh(entity)) {
            if (ImGui::Button("Add static mesh")) {
                m_meshManager->CreateStaticMesh(entity);
            } else {
                return;
            }
        }

        SyncResourcePaths();

        std::vector<const char*> meshPaths;
        meshPaths.reserve(m_meshItems.size());
        for (size_t i = 0; i < m_meshItems.size(); ++i) {
            meshPaths.push_back(m_meshItems[i].path.c_str());
        }

        std::vector<const char*> matPaths;
        matPaths.reserve(m_matItems.size());
        for (size_t i = 0; i < m_matItems.size(); ++i) {
            matPaths.push_back(m_matItems[i].path.c_str());
        }


        // Get active mesh index
        game_StaticMeshId mid         = m_meshManager->GetStaticMeshId(entity);
        const res_Mesh*   mesh        = m_meshManager->GetMesh(mid);
        std::string       curMeshPath = mesh == nullptr ? "Not set" : mesh->GetPath();
        int               curMeshIdx  = 0;
        if (mesh != nullptr) {
            curMeshIdx = -1;
            for (int i = 0; i < meshPaths.size(); ++i) {
                if (curMeshPath == meshPaths[i]) {
                    curMeshIdx = i;
                    break;
                }
            }
        }
        diag_AssertWithReason(curMeshIdx >= 0, "The mesh that was assigned to the entity has been relocated or renamed.");
        if (mesh == nullptr) {
            m_meshManager->SetMesh(mid, m_resources->GetMesh(meshPaths[curMeshIdx]));
        }

        // Mesh dropdown select
        int nextMeshIdx = curMeshIdx;
        if (ImGui::Combo("Mesh", &nextMeshIdx, &meshPaths[0], meshPaths.size())) {
            if (curMeshIdx != nextMeshIdx) {
                m_meshManager->SetMesh(mid, m_resources->GetMesh(meshPaths[nextMeshIdx]));
            }
        }


        // Get active material index
        const res_Material* mat        = m_meshManager->GetMaterial(mid);
        std::string         curMatPath = mat == nullptr ? "Not set" : mat->GetPath();
        int curMatIdx = 0;
        if (mat != nullptr) {
            curMatIdx = -1;
            for (int i = 0; i < matPaths.size(); ++i) {
                if (curMatPath == matPaths[i]) {
                    curMatIdx = i;
                    break;
                }
            }
        }
        diag_AssertWithReason(curMatIdx >= 0, "The material that was assigned to the entity has been relocated or renamed.");
        if (mat == nullptr) {
            m_meshManager->SetMaterial(mid, m_resources->GetMaterial(matPaths[curMatIdx]));
        }

        // Material dropdown select
        int nextMatIdx = curMatIdx;
        if (ImGui::Combo("Material", &nextMatIdx, &matPaths[0], matPaths.size())) {
            if (curMatIdx != nextMatIdx) {
                m_meshManager->SetMaterial(mid, m_resources->GetMaterial(matPaths[nextMatIdx]));
            }
        }
    }
} // namespace pge