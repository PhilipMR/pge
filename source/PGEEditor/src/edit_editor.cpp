#include "../include/edit_editor.h"
#include <game_scene.h>
#include <gfx_render_target.h>
#include <imgui/imgui.h>
#include <input_mouse.h>
#include <gfx_debug_draw.h>
#include <sstream>

namespace pge
{
    static const char* PATH_TO_LAYOUT_INI = "layout.ini";

    edit_Editor::edit_Editor()
        : m_selectedEntity(game_EntityId_Invalid)
        , m_gameWindowSize(1600, 900)
        , m_editMode(edit_EditMode::NONE)
    {
        ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
    }

    static void
    GetAxisVectors(const edit_Axis& axis, math_Vec3* vecsOut, size_t* numVecsOut)
    {
        switch (axis) {
            case edit_Axis::NONE: {
                *numVecsOut = 0;
            } break;
            case edit_Axis::X: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                *numVecsOut = 1;
            } break;
            case edit_Axis::Y: {
                vecsOut[0]  = math_Vec3(0, 1, 0);
                *numVecsOut = 1;
            } break;
            case edit_Axis::Z: {
                vecsOut[0]  = math_Vec3(0, 0, 1);
                *numVecsOut = 1;
            } break;
            case edit_Axis::XY: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                vecsOut[1]  = math_Vec3(0, 1, 0);
                *numVecsOut = 2;
            } break;
            case edit_Axis::XZ: {
                vecsOut[0]  = math_Vec3(1, 0, 0);
                vecsOut[1]  = math_Vec3(0, 0, 1);
                *numVecsOut = 2;
            } break;
            case edit_Axis::YZ: {
                vecsOut[0]  = math_Vec3(0, 1, 0);
                vecsOut[1]  = math_Vec3(0, 0, 1);
                *numVecsOut = 2;
            } break;
            default: {
                diag_AssertWithReason(false, "Unhandled axis!");
            } break;
        }
    }


    static void
    DrawAxis(const game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis)
    {
        auto        tid        = tm->GetTransformId(entity);
        auto        world      = tm->GetWorld(tid);
        auto        pos        = math_Vec3(world[0][3], world[1][3], world[2][3]);
        const float lineLength = 10000.0f;

        math_Vec3 axisVecs[2];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            const math_Vec3& vec = axisVecs[i];
            gfx_DebugDraw_Line(pos - vec * lineLength / 2, pos + vec * lineLength / 2, vec);
        }
    }

    static void
    TranslateEntity(game_TransformManager* tm, const game_Entity& entity, const edit_Axis& axis, const math_Mat4x4& viewProj)
    {
        math_Vec2 delta = input_MouseDelta();
        delta.y *= -1;
        if (math_LengthSquared(delta) == 0)
            return;
        float     deltaMag = math_Length(delta);
        math_Vec2 deltaDir = math_Normalize(delta);

        auto tid   = tm->GetTransformId(entity);
        auto world = tm->GetWorld(tid);
        auto pos   = math_Vec3(world[0][3], world[1][3], world[2][3]);

        math_Vec3 axisVecs[2];
        size_t    numAxisVecs = 0;
        GetAxisVectors(axis, axisVecs, &numAxisVecs);
        for (size_t i = 0; i < numAxisVecs; ++i) {
            math_Vec4 taxis    = viewProj * math_Vec4(axisVecs[i], 0);
            math_Vec2 axisDir  = math_Normalize(math_Vec2(taxis.x, taxis.y));
            float     stepSize = deltaMag * math_Dot(axisDir, deltaDir) * 0.03f;
            pos += stepSize * axisVecs[i];
        }
        world[0][3] = pos.x;
        world[1][3] = pos.y;
        world[2][3] = pos.z;
        tm->SetLocal(tid, world);
    }

    void
    edit_Editor::HandleEvents(game_Scene* scene)
    {
        const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();

        switch (m_editMode) {
            case edit_EditMode::NONE: break;
            case edit_EditMode::TRANSLATE: {
                DrawAxis(scene->GetTransformManager(), m_selectedEntity, m_editAxis);
                TranslateEntity(scene->GetTransformManager(), m_selectedEntity, m_editAxis, viewProj);
                if (input_MouseButtonPressed(input_MouseButton::LEFT))
                    m_editMode = edit_EditMode::NONE;
                if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
                    m_editMode  = edit_EditMode::NONE;
                    auto tid    = scene->GetTransformManager()->GetTransformId(m_selectedEntity);
                    auto world  = scene->GetTransformManager()->GetWorld(tid);
                    world[0][3] = m_preTransformPosition.x;
                    world[1][3] = m_preTransformPosition.y;
                    world[2][3] = m_preTransformPosition.z;
                    scene->GetTransformManager()->SetLocal(tid, world);
                }
                break;
            }
            default: {
                diag_AssertWithReason(false, "Unhandled edit mode!");
            }
        }

        // Left mouse click to (de-)select entity
        if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
            const math_Ray          ray         = math_Raycast_RayFromPixel(input_MousePosition() - m_gameWindowPos, m_gameWindowSize, viewProj);
            const game_StaticMeshId hoveredMesh = scene->GetStaticMeshManager()->GetRaycastStaticMesh(*scene->GetTransformManager(), ray, viewProj);
            if (hoveredMesh != game_StaticMeshId_Invalid) {
                m_selectedEntity = scene->GetStaticMeshManager()->GetEntity(hoveredMesh).id;
            } else {
                m_selectedEntity = game_EntityId_Invalid;
                m_editMode       = edit_EditMode::NONE;
            }
        }

        // Handle mode switch on selected entity
        if (m_selectedEntity != game_EntityId_Invalid) {
            // Draw selected bounding box
            auto meshId      = scene->GetStaticMeshManager()->GetStaticMeshId(m_selectedEntity);
            auto aabb        = scene->GetStaticMeshManager()->GetMesh(meshId)->GetAABB();
            auto transformId = scene->GetTransformManager()->GetTransformId(m_selectedEntity);
            auto world       = scene->GetTransformManager()->GetWorld(transformId);
            aabb             = math_TransformAABB(aabb, world);
            gfx_DebugDraw_Box(aabb.min, aabb.max);

            // Transition to translate-mode (and axis select)
            if (m_editMode != edit_EditMode::TRANSLATE) {
                if (input_KeyboardPressed(input_KeyboardKey::G)) {
                    m_editMode             = edit_EditMode::TRANSLATE;
                    m_editAxis             = edit_Axis::NONE;
                    m_preTransformPosition = math_Vec3(world[0][3], world[1][3], world[2][3]);
                }
            } else {
                if (input_KeyboardDown(input_KeyboardKey::LSHIFT)) {
                    if (input_KeyboardPressed(input_KeyboardKey::X)) {
                        m_editAxis = edit_Axis::YZ;
                    } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                        m_editAxis = edit_Axis::XZ;
                    } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                        m_editAxis = edit_Axis::XY;
                    }
                } else {
                    if (input_KeyboardPressed(input_KeyboardKey::X)) {
                        m_editAxis = edit_Axis::X;
                    } else if (input_KeyboardPressed(input_KeyboardKey::Y)) {
                        m_editAxis = edit_Axis::Y;
                    } else if (input_KeyboardPressed(input_KeyboardKey::Z)) {
                        m_editAxis = edit_Axis::Z;
                    }
                }
            }
        }
    }

    void
    edit_Editor::DrawMenuBar(game_Scene* scene)
    {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save scene", "CTRL+S")) {
                    std::ofstream os("test.scene");
                    os << *scene;
                    os.close();
                }
                if (ImGui::MenuItem("Save scene as...", "CTRL+SHIFT+S")) {}
                if (ImGui::MenuItem("Load scene...", "CTRL+L")) {
                    std::ifstream is("test.scene");
                    is >> *scene;
                    is.close();
                }

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
                    ImGui::SaveIniSettingsToDisk(PATH_TO_LAYOUT_INI);
                }
                if (ImGui::MenuItem("Load layout")) {
                    ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
                    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
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
        bool isHovered   = ImGui::IsWindowHovered();
        m_gameWindowPos  = math_Vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
        m_gameWindowSize = math_Vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
        ImGui::End();
        return isHovered;
    }

    void
    edit_Editor::DrawEntityTree(game_Scene* scene)
    {
        ImGui::Begin("Scene graph", nullptr, 0);
        ImGui::Text("Scene");
        ImGui::Indent();

        bool isAnyNodeHovered = false;
        auto* mm = scene->GetEntityMetaDataManager();
        for (auto it = mm->Begin(); it != mm->End(); ++it) {
            const auto&          entity       = it->second;
            static game_EntityId editEntityId = game_EntityId_Invalid;
            bool                 isSelected   = entity.entity == m_selectedEntity;

            ImGui::Bullet();
            ImGui::SameLine();

            if (editEntityId == entity.entity.id) {
                std::stringstream ss;
                ss << entity.entity.id;
                if (ImGui::InputText(entity.name, (char*)entity.name, sizeof(entity.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    m_selectedEntity = entity.entity.id;
                    editEntityId     = game_EntityId_Invalid;
                }
            } else {
                ImGui::Selectable(entity.name, isSelected);
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    editEntityId = entity.entity.id;
                } else if (ImGui::IsItemClicked()) {
                    if (m_selectedEntity != entity.entity.id) {
                        m_selectedEntity = entity.entity.id;
                        editEntityId     = game_EntityId_Invalid;
                    }
                }
            }

            isAnyNodeHovered |= ImGui::IsItemHovered();
        }

        if (!isAnyNodeHovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_selectedEntity = game_EntityId_Invalid;
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
