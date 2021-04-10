#include "../include/edit_editor.h"
#include "../include/edit_mesh.h"
#include <game_scene.h>
#include <gfx_render_target.h>
#include <imgui/imgui.h>
#include <input_mouse.h>
#include <gfx_debug_draw.h>
#include <sstream>
#include <imgui/IconFontAwesome5.h>

namespace pge
{
    extern void                   edit_BeginFrame();
    extern void                   edit_EndFrame();
    static const char*            PATH_TO_LAYOUT_INI = "layout.ini";
    static const ImGuiWindowFlags PANEL_WINDOW_FLAGS = ImGuiWindowFlags_NoTitleBar;


    edit_Editor::edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_graphicsAdapter(graphicsAdapter)
        , m_graphicsDevice(graphicsDevice)
        , m_resources(resources)
        , m_scene(std::make_unique<game_Scene>(m_graphicsAdapter, m_graphicsDevice, m_resources))
        , m_translator(m_scene->GetTransformManager())
        , m_scaler(m_scene->GetTransformManager())
        , m_rotator(m_scene->GetTransformManager())
        , m_editMode(edit_EditMode::NONE)
        , m_selectedEntity(game_EntityId_Invalid)
        , m_drawGrid(true)
        , m_gameWindowSize(1600, 900)
    {
        ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_TransformEditor(m_scene->GetTransformManager())));
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_MeshEditor(m_scene->GetStaticMeshManager(), resources)));

        auto GetImTexture = [&](const char* path) {
            return (ImTextureID)resources->GetTexture(path)->GetTexture()->GetNativeTexture();
        };
        m_icons.sceneNode         = GetImTexture("data/icons/node.png");
        m_icons.sceneNodeSelected = GetImTexture("data/icons/node_selected.png");
        m_icons.playButton        = GetImTexture("data/icons/btn_play.png");
        m_icons.pauseButton       = GetImTexture("data/icons/btn_pause.png");
        m_icons.pointLight        = resources->GetTexture("data/icons/light_point.png")->GetTexture();
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
        game_PointLightId plightId = scene->GetLightManager()->HoverSelect(*m_scene->GetTransformManager(),
                                                                           hoverPosNorm,
                                                                           billboardSize,
                                                                           scene->GetCamera()->GetViewMatrix(),
                                                                           scene->GetCamera()->GetProjectionMatrix());
        if (plightId != game_PointLightId_Invalid)
            return scene->GetLightManager()->GetPointLight(plightId).entity;

        return game_EntityId_Invalid;
    }

    void
    edit_Editor::DrawGizmos() const
    {
        auto* scene     = m_scene.get();
        auto* resources = m_resources;

        if (m_drawGrid) {
            const float     axisThickness = 0.1f;
            const float     thickness     = 0.025f;
            const float     length        = 10000.0f;
            const math_Vec2 cellSize(5.0f, 5.0f);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(length, 0, 0), math_Vec3(1, 0, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, length, 0), math_Vec3(0, 1, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, 0, length), math_Vec3(0, 0, 1), axisThickness);
            gfx_DebugDraw_GridXY(math_Vec3(0, 0, -thickness), length, cellSize, math_Vec3::One(), thickness);
        }

        // Selected entity AABB
        if (scene->GetStaticMeshManager()->HasStaticMesh(m_selectedEntity) && scene->GetTransformManager()->HasTransform(m_selectedEntity)) {
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
        auto* mm = scene->GetEntityMetaDataManager();
        for (auto it = mm->Begin(); it != mm->End(); ++it) {
            const auto& entity = it->first;
            if (scene->GetLightManager()->HasPointLight(entity)) {
                const auto& plightId = scene->GetLightManager()->GetPointLightId(entity);
                const auto& plight   = scene->GetLightManager()->GetPointLight(plightId);
                math_Vec3   plightPos;
                if (scene->GetTransformManager()->HasTransform(entity)) {
                    auto tid = scene->GetTransformManager()->GetTransformId(entity);
                    plightPos += scene->GetTransformManager()->GetWorldPosition(tid);
                }
                gfx_DebugDraw_Billboard(plightPos, math_Vec2(2, 2), m_icons.pointLight);
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
            } break;

            case edit_EditMode::SCALE: {
                if (m_selectedEntity == game_EntityId_Invalid)
                    break;
                const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();
                m_scaler.UpdateAndDraw(viewProj, input_MouseDelta());
                if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_scaler.CompleteScale(&m_commandStack);
                }
                if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_scaler.CancelScale();
                }
            } break;

            case edit_EditMode::ROTATE: {
                if (m_selectedEntity == game_EntityId_Invalid)
                    break;
                const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();
                m_rotator.UpdateAndDraw(viewProj, input_MouseDelta());
                if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_rotator.CompleteRotation(&m_commandStack);
                }
                if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
                    m_editMode = edit_EditMode::NONE;
                    m_rotator.CancelRotation();
                }
            } break;

            default: {
                diag_AssertWithReason(false, "Unhandled edit mode!");
            } break;
        }

        // Left mouse click to (de-)select entity
        if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
            game_Entity entity = SelectEntity();
            if (entity != m_selectedEntity) {
                m_commandStack.Do(edit_CommandSelectEntity::Create(entity, &m_selectedEntity));
                if (entity == game_EntityId_Invalid) {
                    m_editMode = edit_EditMode::NONE;
                }
                m_translator.CancelTranslation();
                m_scaler.CancelScale();
                m_rotator.CancelRotation();
            }
        }

        // Handle mode switch on selected entity
        if (m_selectedEntity.id != game_EntityId_Invalid) {
            if (!scene->GetTransformManager()->HasTransform(m_selectedEntity))
                return;
            // Transition to translate-mode (and axis select)
            if (m_editMode != edit_EditMode::TRANSLATE) {
                if (input_KeyboardPressed(input_KeyboardKey::G)) {
                    m_translator.CancelTranslation();
                    m_scaler.CancelScale();
                    m_rotator.CancelRotation();

                    m_editMode = edit_EditMode::TRANSLATE;
                    m_translator.BeginTranslation(m_selectedEntity);
                }
            }
            if (m_editMode != edit_EditMode::SCALE) {
                if (input_KeyboardPressed(input_KeyboardKey::S) && !input_MouseButtonDown(input_MouseButton::RIGHT)) {
                    m_translator.CancelTranslation();
                    m_scaler.CancelScale();
                    m_rotator.CancelRotation();

                    m_editMode = edit_EditMode::SCALE;
                    m_scaler.BeginScale(m_selectedEntity);
                }
            }
            if (m_editMode != edit_EditMode::ROTATE) {
                if (input_KeyboardPressed(input_KeyboardKey::R)) {
                    m_translator.CancelTranslation();
                    m_scaler.CancelScale();
                    m_rotator.CancelRotation();

                    m_editMode = edit_EditMode::ROTATE;
                    m_rotator.BeginRotation(m_selectedEntity);
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
                if (ImGui::MenuItem("New scene", "CTRL+N")) {
                    // m_scene.reset(new game_Scene(m_graphicsAdapter, m_graphicsDevice, m_resources));
                }
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
                    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
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


        ImGui::Begin("Game", nullptr, PANEL_WINDOW_FLAGS);

        auto ButtonCenteredOnLine = [&](ImTextureID texture, float alignment = 0.5f) {
            ImGuiStyle& style = ImGui::GetStyle();

            const ImVec2 size(50, 20);
            float        avail = ImGui::GetContentRegionAvail().x;
            float        off   = (avail - size.x) * alignment;
            if (off > 0.0f)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

            return ImGui::ImageButton(texture, size);
        };

        ImGui::Checkbox("Grid", &m_drawGrid);
        ImGui::SameLine();
        static bool isPlaying = false;
        if (!isPlaying) {
            if (ButtonCenteredOnLine(m_icons.playButton)) {
                isPlaying = true;
            }
        } else {
            if (ButtonCenteredOnLine(m_icons.pauseButton)) {
                isPlaying = false;
            }
        }
        const float playBarHeight = ImGui::GetItemRectSize().y;


        float r = 16.0f / 9.0f;
        ImGui::Image(target->GetNativeTexture(), ImVec2(ImGui::GetWindowSize().x - 20, (ImGui::GetWindowSize().y - (playBarHeight + 5)) - 20 * r));
        bool isHovered   = ImGui::IsWindowHovered();
        m_gameWindowPos  = math_Vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + playBarHeight);
        m_gameWindowSize = math_Vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);


        // Right mouse click to open entity context menu
        static bool         mouseDownOnEntity      = false;
        static bool         hoveringSelectedEntity = false;
        static game_Entity  entDown                = game_EntityId_Invalid;
        static double       downStartTime          = 0;
        static const double deleteClickTime        = 0.5;
        if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
            entDown       = SelectEntity();
            downStartTime = ImGui::GetTime();
        }
        if (input_MouseButtonReleased(input_MouseButton::RIGHT) && (ImGui::GetTime() - downStartTime) <= deleteClickTime) {
            game_Entity selected   = SelectEntity();
            hoveringSelectedEntity = (selected.id != game_EntityId_Invalid) && (selected.id == entDown.id) && (selected.id == m_selectedEntity.id);
        }
        hoveringSelectedEntity = hoveringSelectedEntity && math_FloatEqual(math_LengthSquared(input_MouseDelta()), 0);

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

        ImGui::Begin("Scene graph", nullptr, PANEL_WINDOW_FLAGS);

        // ImGui::Image(m_icons.sceneNode, ImVec2(15, 15));
        ImGui::SameLine();
        ImGui::Text("%s Scene", ICON_FA_CARET_SQUARE_DOWN);
        ImGui::Indent();

        bool                     isAnyNodeHovered    = false;
        bool                     isEntityContextMenu = false;
        auto*                    mm                  = scene->GetEntityMetaDataManager();
        std::vector<game_Entity> entitiesToDestroy;
        for (auto it = mm->Begin(); it != mm->End(); ++it) {
            const auto&          entity       = it->second;
            static game_EntityId editEntityId = game_EntityId_Invalid;
            bool                 isSelected   = entity.entity == m_selectedEntity;

            // ImGui::Image(isSelected ? m_icons.sceneNodeSelected : m_icons.sceneNode, ImVec2(15, 15));

            // ImGui::Bullet();
            ImGui::Text("%s", ICON_FA_CARET_RIGHT);
            ImGui::SameLine();

            if (isSelected && editEntityId == entity.entity.id) {
                if (ImGui::InputText("", (char*)entity.name, sizeof(entity.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    m_commandStack.Do(edit_CommandSelectEntity::Create(entity.entity, &m_selectedEntity));
                    editEntityId = game_EntityId_Invalid;
                }
            } else {
                ImGui::Selectable(entity.name, isSelected);
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    editEntityId = entity.entity.id;
                } else if (ImGui::IsItemClicked()) {
                    if (m_selectedEntity.id != entity.entity.id) {
                        m_commandStack.Do(edit_CommandSelectEntity::Create(entity.entity, &m_selectedEntity));
                        editEntityId = game_EntityId_Invalid;
                    }
                }

                std::stringstream ss;
                ss << "SceneGraphEntityContextMenu" << entity.entity.id;
                if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                    isEntityContextMenu = true;
                    if (ImGui::Selectable("Delete entity")) {
                        entitiesToDestroy.push_back(entity.entity);
                    }
                    ImGui::EndPopup();
                }
            }

            isAnyNodeHovered |= ImGui::IsItemHovered();
        }

        for (const auto& entity : entitiesToDestroy) {
            m_commandStack.Do(edit_CommandDeleteEntity::Create(entity, scene));
            if (m_selectedEntity == entity) {
                m_commandStack.Do(edit_CommandSelectEntity::Create(game_EntityId_Invalid, &m_selectedEntity));
            }
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
        ImGui::Begin("Inspector", nullptr, PANEL_WINDOW_FLAGS);
        if (m_selectedEntity.id != game_EntityId_Invalid) {
            if (m_scene->GetLightManager()->HasPointLight(m_selectedEntity)) {
                // lightEditor->UpdateAndDraw(m_selectedEntity);
            } else {
                for (auto& compEditor : m_componentEditors) {
                    compEditor->UpdateAndDraw(m_selectedEntity);
                }
            }
        }
        ImGui::End();
    }

    void
    edit_Editor::DrawExplorer()
    {
        ImGui::Begin("Explorer", nullptr, PANEL_WINDOW_FLAGS);
        ImGui::End();
    }
} // namespace pge
