#include "../include/edit_editor.h"
#include <game_scene.h>
#include <gfx_render_target.h>
#include <imgui/imgui.h>
#include <input_mouse.h>
#include <gfx_debug_draw.h>

namespace pge
{
    static const char* s_PathToLayoutIni = "layout.ini";

    edit_Editor::edit_Editor()
        : m_selectedEntity(game_EntityId_Invalid)
    {
        ImGui::LoadIniSettingsFromDisk(s_PathToLayoutIni);
    }


    void
    edit_Editor::HandleEvents(const game_Scene* scene)
    {
        const math_Vec2         windowSize(1600, 900);
        const math_Mat4x4       viewProj    = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();
        const math_Ray          ray         = math_Raycast_RayFromPixel(input_MousePosition(), windowSize, viewProj);
        const game_StaticMeshId hoveredMesh = scene->GetStaticMeshManager()->GetRaycastStaticMesh(*scene->GetTransformManager(), ray, viewProj);
        if (hoveredMesh != game_StaticMeshId_Invalid) {
            m_selectedEntity = scene->GetStaticMeshManager()->GetEntity(hoveredMesh).id;
            auto aabb = scene->GetStaticMeshManager()->GetMesh(hoveredMesh)->GetAABB();
            auto transformId = scene->GetTransformManager()->GetTransformId(m_selectedEntity);
            auto world = scene->GetTransformManager()->GetWorld(transformId);
            aabb = math_TransformAABB(aabb, world);
            gfx_DebugDraw_Box(aabb.min, aabb.max);
        }
    }

    void
    edit_Editor::DrawMenuBar()
    {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Layout")) {
                if (ImGui::MenuItem("Save layout")) {
                    ImGui::SaveIniSettingsToDisk(s_PathToLayoutIni);
                }
                if (ImGui::MenuItem("Load layout")) {
                    ImGui::LoadIniSettingsFromDisk(s_PathToLayoutIni);
                    ImGui::DockSpaceOverViewport(0, ImGuiDockNodeFlags_PassthruCentralNode);
                }
                ImGui::EndMenu();
            }


            ImGui::EndMainMenuBar();
        }
    }

    bool
    edit_Editor::DrawRenderTarget(const char* title, const gfx_RenderTarget* target)
    {
        ImGui::Begin(title);
        float r = 16.0f / 9.0f;
        ImGui::Image(target->GetNativeTexture(), ImVec2(ImGui::GetWindowSize().x - 20, ImGui::GetWindowSize().y - 20 * r));
        bool isHovered = ImGui::IsWindowHovered();
        ImGui::End();
        return isHovered;
    }

    void
    edit_Editor::DrawEntityTree(const game_Scene* scene)
    {
        const auto* mm = scene->GetEntityMetaDataManager();
        ImGui::Begin("Scene graph", nullptr, 0);
        if (ImGui::TreeNode("Scene")) {
            for (auto it = mm->CBegin(); it != mm->CEnd(); ++it) {
                const auto& entity = it->second;
                std::string name   = entity.name;
                if (ImGui::Button(name.c_str())) {
                    m_selectedEntity = entity.entity.id;
                }
            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

    void
    edit_Editor::DrawInspector(const game_Scene* scene)
    {
        const auto* tm = scene->GetTransformManager();
        const auto* sm = scene->GetStaticMeshManager();
        ImGui::Begin("Inspector", nullptr, 0);
        if (m_selectedEntity != game_EntityId_Invalid) {
            if (tm->HasTransform(m_selectedEntity)) {
                ImGui::Text("HasTransform");
                auto      world = tm->GetWorld(tm->GetTransformId(m_selectedEntity));
                math_Vec3 pos(world[0][3], world[1][3], world[2][3]);
                ImGui::Text("Position: { x:%f, y:%f, z:%f }", pos.x, pos.y, pos.z);
            }
            if (sm->HasStaticMesh(m_selectedEntity)) {
                ImGui::Text("HasStaticMesh");
            }
        }
        ImGui::End();
    }

    void
    edit_Editor::DrawExplorer()
    {
        ImGui::Begin("Explorer", nullptr, 0);
        ImGui::End();
    }
} // namespace pge
