#include "../include/edit_editor.h"
#include "../include/edit_mesh.h"
#include <game_scene.h>
#include <gfx_render_target.h>
#include <imgui/imgui.h>
#include <input_mouse.h>
#include <gfx_debug_draw.h>
#include <sstream>

namespace pge
{
    extern void        edit_BeginFrame();
    extern void        edit_EndFrame();
    static const char* PATH_TO_LAYOUT_INI = "layout.ini";


    edit_Editor::edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_graphicsAdapter(graphicsAdapter)
        , m_graphicsDevice(graphicsDevice)
        , m_resources(resources)
        , m_scene(std::make_unique<game_Scene>(m_graphicsAdapter, m_graphicsDevice, m_resources))
        , m_selectedEntity(game_EntityId_Invalid)
        , m_gameWindowSize(1600, 900)
        , m_translator(m_scene->GetTransformManager())
        , m_editMode(edit_EditMode::NONE)
    {
        ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_TransformEditor(m_scene->GetTransformManager())));
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_MeshEditor(m_scene->GetStaticMeshManager(), resources)));
    }

    void
    edit_Editor::LoadScene(const char* path)
    {
        std::ifstream is(path, std::ios::binary);
        game_Scene&   s = *m_scene;
        is >> s;
        is.close();
    }

    game_Scene&
    edit_Editor::GetScene()
    {
        return *m_scene.get();
    }


    bool
    edit_Editor::UpdateAndDraw(const gfx_RenderTarget* target)
    {
        edit_BeginFrame();

        HandleEvents();
        DrawMenuBar();
        bool ishovering = DrawGameView(target);
        DrawGizmos();
        DrawEntityTree();
        DrawInspector();
        DrawExplorer();

        edit_EndFrame();
        return ishovering;
    }


    game_Entity
    edit_Editor::SelectEntity() const
    {
        auto*             scene    = m_scene.get();
        const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();

        // Static Mesh select
        const math_Ray          ray         = math_Raycast_RayFromPixel(input_MousePosition() - m_gameWindowPos, m_gameWindowSize, viewProj);
        const game_StaticMeshId hoveredMesh = scene->GetStaticMeshManager()->RaycastSelect(*scene->GetTransformManager(), ray, viewProj);
        if (hoveredMesh != game_StaticMeshId_Invalid)
            return scene->GetStaticMeshManager()->GetEntity(hoveredMesh);

        // Point Light select
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
        if (plightId != game_PointLightId_Invalid)
            return scene->GetLightManager()->GetEntity(plightId);

        return game_EntityId_Invalid;
    }

    void
    edit_Editor::DrawGizmos() const
    {
        auto* scene     = m_scene.get();
        auto* resources = m_resources;

        // Selected entity AABB
        if (scene->GetStaticMeshManager()->HasStaticMesh(m_selectedEntity)) {
            auto            meshId = scene->GetStaticMeshManager()->GetStaticMeshId(m_selectedEntity);
            const res_Mesh* mesh   = scene->GetStaticMeshManager()->GetMesh(meshId);
            if (mesh != nullptr) {
                auto transformId = scene->GetTransformManager()->GetTransformId(m_selectedEntity);
                auto world       = scene->GetTransformManager()->GetWorld(transformId);
                auto aabb        = mesh->GetAABB();
                aabb             = math_TransformAABB(aabb, world);
                gfx_DebugDraw_Box(aabb.min, aabb.max);
            }
        }

        // Light billboards
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
    }

    void
    edit_Editor::HandleEvents()
    {
        auto* scene = m_scene.get();

        if (input_KeyboardDown(input_KeyboardKey::CTRL) && input_KeyboardPressed(input_KeyboardKey::Z)) {
            if (input_KeyboardDown(input_KeyboardKey::SHIFT)) {
                m_commandStack.Redo();
            } else {
                m_commandStack.Undo();
            }
        }
        if (input_KeyboardDown(input_KeyboardKey::CTRL) && input_KeyboardPressed(input_KeyboardKey::Y)) {
            m_commandStack.Redo();
        }

        switch (m_editMode) {
            case edit_EditMode::NONE: break;
            case edit_EditMode::TRANSLATE: {
                if (m_selectedEntity == game_EntityId_Invalid)
                    break;
                const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();
                m_translator.UpdateAndDraw(viewProj, input_MouseDelta());
                if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_translator.CompleteTranslation(&m_commandStack);
                }
                if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_translator.CancelTranslation();
                }
                break;
            }
            default: {
                diag_AssertWithReason(false, "Unhandled edit mode!");
            }
        }

        // Left mouse click to (de-)select entity
        if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
            game_Entity entity = SelectEntity();
            m_commandStack.Do(edit_CommandSelectEntity::Create(entity, &m_selectedEntity));
            if (entity == game_EntityId_Invalid) {
                m_editMode = edit_EditMode::NONE;
            }
        }

        // Handle mode switch on selected entity
        if (m_selectedEntity.id != game_EntityId_Invalid) {
            if (!scene->GetTransformManager()->HasTransform(m_selectedEntity))
                return;
            // Transition to translate-mode (and axis select)
            if (m_editMode != edit_EditMode::TRANSLATE) {
                if (input_KeyboardPressed(input_KeyboardKey::G)) {
                    m_editMode = edit_EditMode::TRANSLATE;
                    m_translator.BeginTranslation(m_selectedEntity);
                }
            }
        }
    }

    void
    edit_Editor::DrawMenuBar()
    {
        auto* scene = m_scene.get();

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
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                    m_commandStack.Undo();
                }
                if (ImGui::MenuItem("Redo", "CTRL+Y")) {
                    m_commandStack.Redo();
                }
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
    edit_Editor::DrawGameView(const gfx_RenderTarget* target)
    {
        auto* scene = m_scene.get();
        ImGui::Begin("Game");

        //        static bool isPlaying = false;
        //        if (!isPlaying) {
        //            if (ImGui::Button("PLAY")) {
        //                isPlaying = true;
        //            }
        //        } else {
        //            if (ImGui::Button("PAUSE")) {
        //                isPlaying = false;
        //            }
        //        }
        const float playBarHeight = ImGui::GetItemRectSize().y;


        float r = 16.0f / 9.0f;
        ImGui::Image(target->GetNativeTexture(), ImVec2(ImGui::GetWindowSize().x - 20, ImGui::GetWindowSize().y - 20 * r));
        bool isHovered   = ImGui::IsWindowHovered();
        m_gameWindowPos  = math_Vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + playBarHeight);
        m_gameWindowSize = math_Vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);


        // Right mouse click to open entity context menu
        static bool        mouseDownOnEntity      = false;
        static bool        hoveringSelectedEntity = false;
        static game_Entity entDown                = game_EntityId_Invalid;
        if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
            entDown = SelectEntity();
        }
        if (input_MouseButtonReleased(input_MouseButton::RIGHT)) {
            game_Entity selected   = SelectEntity();
            hoveringSelectedEntity = (selected.id != game_EntityId_Invalid) && (selected.id == entDown.id) && (selected.id == m_selectedEntity.id);
        }

        static bool contextWasOpen = false;
        if (hoveringSelectedEntity) {
            std::stringstream ss;
            ss << "GameEntityContextMenu" << m_selectedEntity.id;
            if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                contextWasOpen = true;
                if (ImGui::Selectable("Delete entity")) {
                    m_commandStack.Do(edit_CommandDeleteEntity::Create(m_selectedEntity, scene));
                    m_selectedEntity.id = game_EntityId_Invalid;
                }
                ImGui::EndPopup();
            } else if (contextWasOpen) {
                hoveringSelectedEntity = false;
                contextWasOpen         = false;
            }
        }

        ImGui::End();
        return isHovered;
    }

    void
    edit_Editor::DrawEntityTree()
    {
        auto* scene = m_scene.get();

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
                    m_commandStack.Do(edit_CommandSelectEntity::Create(entity.entity, &m_selectedEntity));
                    editEntityId     = game_EntityId_Invalid;
                }
            } else {
                ImGui::Selectable(entity.name, isSelected);
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    editEntityId = entity.entity.id;
                } else if (ImGui::IsItemClicked()) {
                    if (m_selectedEntity.id != entity.entity.id) {
                        m_commandStack.Do(edit_CommandSelectEntity::Create(entity.entity, &m_selectedEntity));
                        editEntityId        = game_EntityId_Invalid;
                    }
                }

                std::stringstream ss;
                ss << "SceneGraphEntityContextMenu" << entity.entity.id;
                if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                    isEntityContextMenu = true;
                    if (ImGui::Selectable("Delete entity")) {
                        m_commandStack.Do(edit_CommandDeleteEntity::Create(entity.entity, scene));
                        if (m_selectedEntity == entity.entity.id) {
                            m_commandStack.Do(edit_CommandSelectEntity::Create(game_EntityId_Invalid, &m_selectedEntity));
                        }
                    }
                    ImGui::EndPopup();
                }
            }

            isAnyNodeHovered |= ImGui::IsItemHovered();
        }

        if (!isAnyNodeHovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_commandStack.Do(edit_CommandSelectEntity::Create(game_EntityId_Invalid, &m_selectedEntity));
        }

        if (!isEntityContextMenu && ImGui::BeginPopupContextWindow("SceneContextMenu")) {
            if (ImGui::Selectable("Create entity")) {
                m_commandStack.Do(edit_CommandCreateEntity::Create(scene->GetEntityManager(), scene->GetEntityMetaDataManager()));
            }
            if (ImGui::Selectable("Create light (directional)")) {
                m_commandStack.Do(edit_CommandCreatePointLight::Create(scene->GetEntityManager(),
                                                                       scene->GetEntityMetaDataManager(),
                                                                       scene->GetTransformManager(),
                                                                       scene->GetLightManager()));
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void
    edit_Editor::DrawInspector()
    {
        ImGui::Begin("Inspector", nullptr, 0);
        if (m_selectedEntity.id != game_EntityId_Invalid) {
            for (auto& compEditor : m_componentEditors) {
                compEditor->UpdateAndDraw(m_selectedEntity);
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
