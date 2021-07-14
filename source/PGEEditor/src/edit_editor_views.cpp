#include "../include/edit_editor_views.h"
#include "../include/edit_entity.h"
#include "../include/edit_camera.h"
#include "../include/edit_light.h"

#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <input_mouse.h>
#include <imgui/IconFontAwesome5.h>
#include <imgui/imgui.h>
#include <sstream>

namespace pge
{
    void
    edit_DrawMainMenuBar(game_World* world, edit_CommandStack* cstack)
    {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New world", "CTRL+N")) {}
                if (ImGui::MenuItem("Save world", "CTRL+S")) {
                    std::ofstream os("test.world");
                    os << *world;
                    os.close();
                }
                if (ImGui::MenuItem("Save world as...", "CTRL+SHIFT+S")) {}
                if (ImGui::MenuItem("Load world...", "CTRL+L")) {
                    std::ifstream is("test.world");
                    is >> *world;
                    is.close();
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                    cstack->Undo();
                }
                if (ImGui::MenuItem("Redo", "CTRL+Y")) {
                    cstack->Redo();
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

    void
    edit_DrawGameViewMenu(bool* isPlay, bool* drawGrid, bool* drawGizmos)
    {
        float barWidth = ImGui::GetContentRegionAvailWidth();
        if (*isPlay == false) {
            ImGui::Checkbox("Grid", drawGrid);
            ImGui::SameLine();
            ImGui::Checkbox("Gizmos", drawGizmos);
            ImGui::SameLine();
        }

        const ImVec2 size(50, 20);
        ImGui::SetCursorPosX(barWidth - size.x);
        if (*isPlay == false) {
            if (ImGui::Button(ICON_FA_PLAY, size)) {
                *isPlay = true;
            }
        } else {
            if (ImGui::Button(ICON_FA_PAUSE, size)) {
                *isPlay = false;
            }
        }
    }

    void
    edit_DrawLogView()
    {
#undef ERROR
        const std::uint8_t logDebugFlag   = 1 << core_LogRecord::RecordType::DEBUG;
        const std::uint8_t logWarningFlag = 1 << core_LogRecord::RecordType::WARNING;
        const std::uint8_t logErrorFlag   = 1 << core_LogRecord::RecordType::ERROR;

        static std::uint8_t logMask = logDebugFlag | logWarningFlag | logErrorFlag;

        if (ImGui::Button("Clear")) {
            core_ClearLogRecords();
        }
        ImGui::SameLine();

        bool showLogDebug = logMask & logDebugFlag;
        if (ImGui::Checkbox("Debug", &showLogDebug)) {
            logMask ^= logDebugFlag;
        }
        ImGui::SameLine();

        bool showLogWarning = logMask & logWarningFlag;
        if (ImGui::Checkbox("Warning", &showLogWarning)) {
            logMask ^= logWarningFlag;
        }
        ImGui::SameLine();

        bool showLogError = logMask & logErrorFlag;
        if (ImGui::Checkbox("Error", &showLogError)) {
            logMask ^= logErrorFlag;
        }

        ImGui::BeginChild("LogText", ImVec2(0, 0), true);
        auto records = core_GetLogRecords();
        for (const auto& record : records) {
            if (logMask & (1 << record.type)) {
                ImGui::Text("%s", record.message.c_str());
            }
        }
        static size_t lastNumRecs = 0;
        if (lastNumRecs != records.size()) {
            ImGui::SetScrollHereY(1.0f);
            lastNumRecs = records.size();
        }
        ImGui::EndChild();
    }



    // ========================================
    // edit_GameView
    // ========================================
    edit_GameView::edit_GameView(gfx_GraphicsAdapter* graphicsAdapter, res_ResourceManager* resources, unsigned width, unsigned height)
        : m_graphicsAdapter(graphicsAdapter)
        , m_rtGameMs(graphicsAdapter, width, height, true, true)
        , m_rtGame(graphicsAdapter, width, height, false, false)
        , m_multisampleEffect(resources->GetEffect("data\\effects\\multisample.effect"))
    {}

    void
    edit_GameView::DrawOnGUI(game_World* world, const math_Vec2& size, const math_Mat4x4& view, const math_Mat4x4& proj)
    {
        const gfx_RenderTarget* prevRt = gfx_RenderTarget_GetActiveRTV();

        // Draw world to texture
        gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
        m_rtGameMs.Bind();
        m_rtGameMs.Clear();
        world->Draw(view, proj);

        // Redraw to non-multisampling texture (for ImGui)
        gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
        m_rtGame.Bind();
        m_rtGame.Clear();
        world->GetRenderer()->DrawRenderToView(&m_rtGameMs, m_multisampleEffect);

        if (prevRt == nullptr) {
            gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
        } else {
            prevRt->Bind();
        }
        ImGui::Image(m_rtGame.GetNativeTexture(), ImVec2(size.x, size.y));
    }


    // ========================================
    // edit_EntityHierarchyView
    // ========================================
    void
    edit_EntityHierarchyView::ApplyDrop(game_World* world, const game_Entity& targetEntity)
    {
        const game_Entity& droppedEntity = *(game_Entity*)ImGui::GetDragDropPayload()->Data;
        game_TransformId   srcTransform  = world->GetTransformManager()->GetTransformId(droppedEntity);
        game_TransformId   destTransform
            = targetEntity == game_EntityId_Invalid ? game_TransformId_Invalid : world->GetTransformManager()->GetTransformId(targetEntity);
        if (destTransform != game_TransformId_Invalid && world->GetTransformManager()->IsAncestor(destTransform, srcTransform)) {
            // No cycles allowed.
            return;
        }
        world->GetTransformManager()->SetParent(srcTransform, destTransform);
    }

    void
    edit_EntityHierarchyView::DestroyRecursive(game_World* world, const game_Entity& entity, std::vector<game_Entity>* entitiesRemove)
    {
        auto* tmanager = world->GetTransformManager();
        auto  tid      = tmanager->GetTransformId(entity);
        auto  child    = tmanager->GetFirstChild(tid);
        while (child != game_TransformId_Invalid) {
            DestroyRecursive(world, tmanager->GetEntity(child), entitiesRemove);
            child = tmanager->GetNextSibling(child);
        }
        entitiesRemove->push_back(entity);
    }

    bool
    edit_EntityHierarchyView::DrawEntityNode(game_World*               world,
                                             const game_Entity&        entity,
                                             game_Entity*              selectedEntity,
                                             std::vector<game_Entity>* entitiesRemove,
                                             bool                      isLeaf)
    {
        std::string        name  = world->GetEntityManager()->GetName(entity);
        ImGuiTreeNodeFlags flags = 0;
        flags |= isLeaf ? (ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen) : 0;
        flags |= (entity == *selectedEntity) ? ImGuiTreeNodeFlags_Selected : 0;
        bool isOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity.id), flags, "%s", name.c_str());

        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("_TREENODE", (void*)&entity, sizeof(entity));
            ImGui::Text("Drag onto node to set parent");
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            ImGui::AcceptDragDropPayload("_TREENODE");
            if (ImGui::GetDragDropPayload()->IsDelivery()) {
                ApplyDrop(world, entity);
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::IsItemHovered())
            m_hoveredEntity = entity;
        if (ImGui::IsItemClicked())
            *selectedEntity = entity;


        if (m_hoveredEntity == entity) {
            bool              destroy = false;
            std::stringstream cmenuId;
            cmenuId << "EntityContextMenu" << entity.id;
            if (ImGui::BeginPopupContextWindow(cmenuId.str().c_str())) {
                if (ImGui::Selectable("Delete entity")) {
                    DestroyRecursive(world, entity, entitiesRemove);
                    destroy = true;
                }
                ImGui::EndPopup();
            }
            if (!isLeaf && isOpen && destroy) {
                ImGui::TreePop();
                return false;
            }
        }

        return isOpen;
    };

    void
    edit_EntityHierarchyView::DrawLocalTree(game_World*               world,
                                            const game_Entity&        entity,
                                            game_Entity*              selectedEntity,
                                            std::vector<game_Entity>* entitiesRemove)
    {
        const game_EntityManager* emanager = world->GetEntityManager();
        game_TransformManager*    tmanager = world->GetTransformManager();
        auto                      tid      = tmanager->GetTransformId(entity);
        auto                      child    = tmanager->GetFirstChild(tid);
        if (child == game_TransformId_Invalid) {
            DrawEntityNode(world, entity, selectedEntity, entitiesRemove, true);
        } else {
            bool isOpen = DrawEntityNode(world, entity, selectedEntity, entitiesRemove, false);
            if (isOpen) {
                while (child != game_TransformId_Invalid) {
                    game_Entity childEnt = tmanager->GetEntity(child);
                    if (emanager->IsEntityAlive(childEnt)) {
                        DrawLocalTree(world, childEnt, selectedEntity, entitiesRemove);
                    }
                    child = tmanager->GetNextSibling(child);
                }
                ImGui::TreePop();
            }
        }
    }

    void
    edit_EntityHierarchyView::DrawOnGUI(game_World* world, game_Entity* selectedEntity, edit_CommandStack* cstack)
    {
        ImGui::Text("%s World", ICON_FA_CARET_SQUARE_DOWN);

        const game_EntityManager&    emanager = *world->GetEntityManager();
        const game_TransformManager* tmanager = world->GetTransformManager();

        // None hovered unless set by the node views
        std::vector<game_Entity> entitiesRemove;
        for (const auto& entity : emanager) {
            if (tmanager->HasTransform(entity)) {
                auto tid = tmanager->GetTransformId(entity);
                if (tmanager->GetParent(tid) != game_TransformId_Invalid) {
                    // Will be drawn in one of the local trees instead.
                    continue;
                } else {
                    DrawLocalTree(world, entity, selectedEntity, &entitiesRemove);
                }
            } else {
                DrawEntityNode(world, entity, selectedEntity, &entitiesRemove, true);
            }
        }

        for (const auto& ent : entitiesRemove) {
            world->GetEntityManager()->DestroyEntity(ent);
        }

        if (!ImGui::IsAnyItemHovered() && ImGui::IsAnyMouseDown()) {
            m_hoveredEntity = game_EntityId_Invalid;
        }
        bool anyEntityHovered = m_hoveredEntity != game_EntityId_Invalid;
        if (!anyEntityHovered) {
            if (ImGui::IsWindowHovered()) {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    // Deselect if non-node was clicked (within window)
                    *selectedEntity = game_EntityId_Invalid;
                } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    // (Drag-)Drop to root (drop on non-node within window)
                    if (ImGui::GetDragDropPayload() != nullptr) {
                        ApplyDrop(world, game_EntityId_Invalid);
                    }
                }
            }

            // Context menu for non-node area
            if (ImGui::BeginPopupContextWindow("WorldContextMenu")) {
                if (ImGui::Selectable("Create entity")) {
                    auto create = edit_CommandCreateEntity::Create(world);
                    create->Do();

                    // Add transform to entity so that it can be parented.
                    // TODO: CreateTransform command
                    auto newEnt = ((edit_CommandCreateEntity*)create.get())->GetCreatedEntity();
                    world->GetTransformManager()->CreateTransform(newEnt);

                    cstack->Add(std::move(create));
                }
                if (ImGui::Selectable("Create camera")) {
                    cstack->Do(edit_CommandCreateCamera::Create(world));
                }
                if (ImGui::Selectable("Create light (directional)")) {
                    cstack->Do(edit_CommandCreateDirectionalLight::Create(world));
                }
                if (ImGui::Selectable("Create light (point)")) {
                    cstack->Do(edit_CommandCreatePointLight::Create(world));
                }
                ImGui::EndPopup();
            }
        }
    }
} // namespace pge