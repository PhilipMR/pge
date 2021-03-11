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
                if (m_selectedEntity == game_EntityId_Invalid)
                    break;
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
            const game_StaticMeshId hoveredMesh = scene->GetStaticMeshManager()->RaycastSelect(*scene->GetTransformManager(), ray, viewProj);

            math_Vec2 hoverPosNorm(input_MousePosition());
            hoverPosNorm.x -= m_gameWindowPos.x;
            hoverPosNorm.y -= m_gameWindowPos.y;
            hoverPosNorm.x /= m_gameWindowSize.x;
            hoverPosNorm.y /= m_gameWindowSize.y;

            math_Vec2         billboardSize(2, 2);
            game_PointLightId plightId = scene->GetLightManager()->HoverSelect(*scene->GetTransformManager(),
                                                                               hoverPosNorm,
                                                                               billboardSize,
                                                                               scene->GetCamera()->GetViewMatrix(),
                                                                               scene->GetCamera()->GetProjectionMatrix());

            if (plightId != game_PointLightId_Invalid) {
                m_selectedEntity = scene->GetLightManager()->GetEntity(plightId).id;
            } else if (hoveredMesh != game_StaticMeshId_Invalid) {
                m_selectedEntity = scene->GetStaticMeshManager()->GetEntity(hoveredMesh).id;
            } else {
                m_selectedEntity = game_EntityId_Invalid;
                m_editMode       = edit_EditMode::NONE;
            }
        }

        // Handle mode switch on selected entity
        if (m_selectedEntity != game_EntityId_Invalid) {
            if (!scene->GetTransformManager()->HasTransform(m_selectedEntity))
                return;

            auto transformId = scene->GetTransformManager()->GetTransformId(m_selectedEntity);
            auto world       = scene->GetTransformManager()->GetWorld(transformId);

            // Draw selected bounding box
            if (scene->GetStaticMeshManager()->HasStaticMesh(m_selectedEntity)) {
                auto meshId = scene->GetStaticMeshManager()->GetStaticMeshId(m_selectedEntity);
                auto aabb   = scene->GetStaticMeshManager()->GetMesh(meshId)->GetAABB();
                aabb        = math_TransformAABB(aabb, world);
                gfx_DebugDraw_Box(aabb.min, aabb.max);
            }

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
    edit_Editor::DrawGameView(game_Scene* scene, const gfx_RenderTarget* target, res_ResourceManager* resources)
    {
        ImGui::Begin("Game");

        static bool isPlaying = false;
        if (!isPlaying) {
            if (ImGui::Button("PLAY")) {
                isPlaying = true;
            }
        } else {
            if (ImGui::Button("PAUSE")) {
                isPlaying = false;
            }
        }
        const float playBarHeight = ImGui::GetItemRectSize().y;


        float r = 16.0f / 9.0f;
        ImGui::Image(target->GetNativeTexture(), ImVec2(ImGui::GetWindowSize().x - 20, ImGui::GetWindowSize().y - 20 * r));
        bool isHovered   = ImGui::IsWindowHovered();
        m_gameWindowPos  = math_Vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + playBarHeight);
        m_gameWindowSize = math_Vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);


        // Right mouse click to open entity context menu
        static bool hoveringSelectedEntity = false;
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            const math_Mat4x4       viewProj    = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();
            const math_Ray          ray         = math_Raycast_RayFromPixel(input_MousePosition() - m_gameWindowPos, m_gameWindowSize, viewProj);
            const game_StaticMeshId hoveredMesh = scene->GetStaticMeshManager()->RaycastSelect(*scene->GetTransformManager(), ray, viewProj);
            if (hoveredMesh != game_StaticMeshId_Invalid) {
                game_EntityId hoveredEntity = scene->GetStaticMeshManager()->GetEntity(hoveredMesh).id;
                hoveringSelectedEntity      = hoveredEntity == m_selectedEntity;
            } else {
                hoveringSelectedEntity = false;
            }
        }

        static bool contextWasOpen = false;
        if (hoveringSelectedEntity) {
            std::stringstream ss;
            ss << "GameEntityContextMenu" << m_selectedEntity;
            if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                contextWasOpen = true;
                if (ImGui::Selectable("Delete entity")) {
                    scene->GetEntityManager()->DestroyEntity(m_selectedEntity);
                    m_selectedEntity = game_EntityId_Invalid;
                }
                ImGui::EndPopup();
            } else if (contextWasOpen) {
                hoveringSelectedEntity = false;
                contextWasOpen         = false;
            }
        }


        static const gfx_Texture2D* testTex = resources->GetTexture("data/materials/checkers.png")->GetTexture();
        auto*                       mm      = scene->GetEntityMetaDataManager();
        for (auto it = mm->Begin(); it != mm->End(); ++it) {
            const auto& entity = it->first;
            if (scene->GetLightManager()->HasPointLight(entity)) {
                const auto& plightId = scene->GetLightManager()->GetPointLightId(entity);
                const auto& plight   = scene->GetLightManager()->GetPointLight(plightId);
                math_Vec3   plightPos;
                if (scene->GetTransformManager()->HasTransform(entity)) {
                    auto tid   = scene->GetTransformManager()->GetTransformId(entity);
                    auto world = scene->GetTransformManager()->GetWorld(tid);
                    plightPos += math_Vec3(world[0][3], world[1][3], world[2][3]);
                }
                gfx_DebugDraw_Billboard(plightPos, math_Vec2(2, 2), testTex);
            }
        }

        ImGui::End();
        return isHovered;
    }

    void
    edit_Editor::DrawEntityTree(game_Scene* scene)
    {
        ImGui::Begin("Scene graph", nullptr, 0);

        ImGui::Text("Scene");
        ImGui::Indent();

        bool                     isAnyNodeHovered    = false;
        bool                     isEntityContextMenu = false;
        auto*                    mm                  = scene->GetEntityMetaDataManager();
        std::vector<game_Entity> entitiesToDestroy;
        for (auto it = mm->Begin(); it != mm->End(); ++it) {
            const auto&          entity       = it->second;
            static game_EntityId editEntityId = game_EntityId_Invalid;
            bool                 isSelected   = entity.entity == m_selectedEntity;

            ImGui::Bullet();
            ImGui::SameLine();

            if (isSelected && editEntityId == entity.entity.id) {
                std::stringstream ss;
                ss << entity.entity.id;
                if (ImGui::InputText(ss.str().c_str(), (char*)entity.name, sizeof(entity.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
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

                std::stringstream ss;
                ss << "SceneGraphEntityContextMenu" << entity.entity.id;
                if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                    isEntityContextMenu = true;
                    if (ImGui::Selectable("Delete entity")) {
                        entitiesToDestroy.push_back(entity.entity);
                        if (m_selectedEntity == entity.entity.id) {
                            m_selectedEntity = game_EntityId_Invalid;
                        }
                    }
                    ImGui::EndPopup();
                }
            }

            isAnyNodeHovered |= ImGui::IsItemHovered();
        }

        if (!isAnyNodeHovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_selectedEntity = game_EntityId_Invalid;
        }

        if (!isEntityContextMenu && ImGui::BeginPopupContextWindow("SceneContextMenu")) {
            if (ImGui::Selectable("Create entity")) {
                auto                newEntity = scene->GetEntityManager()->CreateEntity();
                game_EntityMetaData meta;
                meta.entity = newEntity;
                std::stringstream ss;
                ss << "Entity [" << newEntity.id << "]";
                strcpy_s(meta.name, ss.str().c_str());
                scene->GetEntityMetaDataManager()->CreateMetaData(newEntity, meta);
            }
            if (ImGui::Selectable("Create light (directional)")) {
                auto                newEntity = scene->GetEntityManager()->CreateEntity();
                game_EntityMetaData meta;
                meta.entity = newEntity;
                std::stringstream ss;
                ss << "DirLight [" << newEntity.id << "]";
                strcpy_s(meta.name, ss.str().c_str());
                scene->GetEntityMetaDataManager()->CreateMetaData(newEntity, meta);

                scene->GetTransformManager()->CreateTransform(newEntity);

                game_PointLight plight;
                plight.radius   = 10.0f;
                plight.strength = 1.0f;
                plight.color    = math_Vec3::One();
                scene->GetLightManager()->CreatePointLight(newEntity, plight);
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        for (const auto& entity : entitiesToDestroy) {
            scene->GetEntityManager()->DestroyEntity(entity);
        }
    }

    void
    edit_Editor::DrawInspector(game_Scene* scene, res_ResourceManager* resources)
    {
        auto* tm = scene->GetTransformManager();
        auto* sm = scene->GetStaticMeshManager();
        ImGui::Begin("Inspector", nullptr, 0);
        if (m_selectedEntity != game_EntityId_Invalid) {
            if (tm->HasTransform(m_selectedEntity)) {
                auto      tid   = tm->GetTransformId(m_selectedEntity);
                auto      world = tm->GetWorld(tid);
                math_Vec3 pos(world[0][3], world[1][3], world[2][3]);
                if (ImGui::DragFloat3("Position", &pos[0])) {
                    world[0][3] = pos[0];
                    world[1][3] = pos[1];
                    world[2][3] = pos[2];
                    tm->SetLocal(tid, world);
                }
            } else {
                if (ImGui::Button("Add transform")) {
                    tm->CreateTransform(m_selectedEntity);
                }
            }
            if (sm->HasStaticMesh(m_selectedEntity)) {
                game_StaticMeshId mid = sm->GetStaticMeshId(m_selectedEntity);
                ImGui::Text("%s", sm->GetMesh(mid)->GetPath().c_str());
            } else {
                if (ImGui::Button("Add mesh")) {
                    if (!tm->HasTransform(m_selectedEntity)) {
                        tm->CreateTransform(m_selectedEntity);
                    }
                    game_StaticMeshId mid = sm->CreateStaticMesh(m_selectedEntity);
                    sm->SetMesh(mid, resources->GetMesh("data/meshes/cube/Cube.001.mesh"));
                    sm->SetMaterial(mid, resources->GetMaterial("data/materials/checkers.mat"));
                }
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
