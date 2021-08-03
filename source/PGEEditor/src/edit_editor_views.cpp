#include "../include/edit_editor_views.h"
#include "../include/edit_mesh.h"
#include "../include/edit_script.h"

#include <core_file_utils.h>
#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <input_mouse.h>
#include <imgui/IconFontAwesome5.h>
#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
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
                    cstack->Clear();
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
                    ImGui::SaveIniSettingsToDisk(edit_PATH_TO_LAYOUT_INI);
                }
                if (ImGui::MenuItem("Load layout")) {
                    ImGui::LoadIniSettingsFromDisk(edit_PATH_TO_LAYOUT_INI);
                    ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);
                }
                ImGui::EndMenu();
            }


            ImGui::EndMainMenuBar();
        }
    }


    // ========================================
    // edit_LogView
    // ========================================
    void
    edit_LogView::DrawOnGUI()
    {
        if (ImGui::Button("Clear")) {
            core_ClearLogRecords();
        }
        ImGui::SameLine();

        bool showLogDebug = m_logMask & LOG_FLAG_DEBUG;
        if (ImGui::Checkbox("Debug", &showLogDebug)) {
            m_logMask ^= LOG_FLAG_DEBUG;
        }
        ImGui::SameLine();

        bool showLogWarning = m_logMask & LOG_FLAG_WARNING;
        if (ImGui::Checkbox("Warning", &showLogWarning)) {
            m_logMask ^= LOG_FLAG_WARNING;
        }
        ImGui::SameLine();

        bool showLogError = m_logMask & LOG_FLAG_ERROR;
        if (ImGui::Checkbox("Error", &showLogError)) {
            m_logMask ^= LOG_FLAG_ERROR;
        }

        ImGui::BeginChild("LogText", ImVec2(0, 0), true);
        auto records = core_GetLogRecords();
        for (const auto& record : records) {
            if (m_logMask & (1 << record.type)) {
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
    // edit_GizmoView
    // ========================================
    edit_GizmoView::edit_GizmoView(game_World* world, res_ResourceManager* resources, edit_CommandStack* cstack)
        : m_transformGizmo(world->GetTransformManager(), cstack)
        , m_world(world)
        , m_lightIcon(resources->GetTexture("data/icons/light_point.png")->GetTexture())
        , m_cameraIcon(resources->GetTexture("data/icons/camera.png")->GetTexture())
    {}

    bool
    edit_GizmoView::IsActive() const
    {
        return ImGuizmo::IsOver() && m_transformGizmo.IsVisible();
    }

    void
    edit_GizmoView::UpdateAndDraw(bool               drawGrid,
                                  bool               drawGizmos,
                                  const game_Entity& selectedEntity,
                                  const math_Vec2&   windowPos,
                                  const math_Vec2&   windowSize,
                                  const math_Mat4x4& view,
                                  const math_Mat4x4& proj)
    {
        if (drawGrid) {
            const float     axisThickness = 0.1f;
            const float     thickness     = 0.025f;
            const float     length        = 1000.0f;
            const math_Vec2 cellSize(5.0f, 5.0f);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(length, 0, 0), math_Vec3(1, 0, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, length, 0), math_Vec3(0, 1, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, 0, length), math_Vec3(0, 0, 1), axisThickness);
            gfx_DebugDraw_GridXY(math_Vec3(0, 0, -thickness), length, cellSize, math_Vec3::One() * 0.5f, thickness);
        }

        ImGuizmo::Enable(drawGizmos);
        if (drawGizmos) {
            const game_EntityManager&    emanager  = *m_world->GetEntityManager();
            const game_MeshManager*      smmanager = m_world->GetMeshManager();
            const game_TransformManager* tmanager  = m_world->GetTransformManager();
            const game_LightManager*     lmanager  = m_world->GetLightManager();
            const game_CameraManager*    cmanager  = m_world->GetCameraManager();

            ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
            m_transformGizmo.TransformEntity(selectedEntity, view, proj);

            // Selected entity AABB
            if (smmanager->HasMesh(selectedEntity) && tmanager->HasTransform(selectedEntity)) {
                auto            meshId = smmanager->GetMeshId(selectedEntity);
                const res_Mesh* mesh   = smmanager->GetMesh(meshId);
                if (mesh != nullptr) {
                    auto transformId = tmanager->GetTransformId(selectedEntity);
                    auto xform       = tmanager->GetWorldMatrix(transformId);
                    auto aabb        = mesh->GetAABB();
                    aabb             = math_TransformAABB(aabb, xform);
                    gfx_DebugDraw_Box(aabb.min, aabb.max);
                }
            }

            // Light and Camera billboards
            for (const auto& entity : emanager) {
                if (!tmanager->HasTransform(entity))
                    continue;

                const auto      tid      = tmanager->GetTransformId(entity);
                const math_Vec3 worldPos = tmanager->GetWorldPosition(tid);

                const bool isDirLight   = lmanager->HasDirectionalLight(entity);
                const bool isPointLight = lmanager->HasPointLight(entity);
                const bool isCamera     = cmanager->HasCamera(entity);

                if (isDirLight || isPointLight || isCamera) {
                    gfx_DebugDraw_Billboard(worldPos, math_Vec2(2, 2), isCamera ? m_cameraIcon : m_lightIcon);
                }

                if (isDirLight && entity == selectedEntity) {
                    auto                         lid             = lmanager->GetDirectionalLightId(entity);
                    const game_DirectionalLight& light           = lmanager->GetDirectionalLight(lid);
                    constexpr float              DIR_DRAW_LENGTH = 5.0f;
                    const math_Vec3              lightDir = math_Normalize((tmanager->GetWorldMatrix(tid) * math_Vec4(light.direction, 0)).xyz);
                    gfx_DebugDraw_Line(worldPos, worldPos + lightDir * DIR_DRAW_LENGTH, light.color);
                }

                if (isCamera && entity == selectedEntity) {
                    game_PerspectiveInfo perspective = cmanager->GetPerspective(entity);
                    const math_Mat4x4    camMatrix   = tmanager->GetLocalMatrix(tid);

                    // Draw the frustrum one unit behind the actual camera to prevent it from
                    // obstructing its view.
                    game_DebugDraw_Frustum(perspective, worldPos - camMatrix.Forward(), camMatrix);
                }
            }
        }
    }


    // ========================================
    // edit_GameView
    // ========================================
    edit_EditView::edit_EditView(gfx_GraphicsAdapter* graphicsAdapter,
                                 res_ResourceManager* resources,
                                 game_World*          world,
                                 edit_CommandStack*   cstack,
                                 unsigned             width,
                                 unsigned             height)
        : m_graphicsAdapter(graphicsAdapter)
        , m_world(world)
        , m_cstack(cstack)
        , m_gizmoView(world, resources, cstack)
        , m_rtGameMs(graphicsAdapter, width, height, true, true)
        , m_rtGame(graphicsAdapter, width, height, false, false)
        , m_multisampleEffect(resources->GetEffect("data\\effects\\multisample.effect"))
    {}

    game_Entity
    edit_EditView::EntityAtCursor() const
    {
        const math_Mat4x4& view = m_editCamera.GetView();
        const math_Mat4x4& proj = m_editCamera.GetProjection();

        math_Vec2 cursor = math_Vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y) - m_viewPos;
        return m_world->FindEntityAtCursor(cursor, m_viewSize, view, proj);
    }

    static void
    DrawGameViewMenu(bool* isPlay, bool* drawGrid, bool* drawGizmos, game_RenderPass* drawPass)
    {
        // Checkboxes
        float barWidth = ImGui::GetContentRegionAvailWidth();
        if (*isPlay == false) {
            ImGui::Checkbox("Grid", drawGrid);
            ImGui::SameLine();
            ImGui::Checkbox("Gizmos", drawGizmos);
        }

        // Draw render pass selection
        {
            ImGui::SameLine();
            ImGui::BeginChild("Render", ImVec2(350, 40), true);

            bool isSelected = *drawPass == game_RenderPass::DEPTH;
            if (ImGui::Selectable("Depth", &isSelected, 0, ImVec2(80, 20))) {
                *drawPass = game_RenderPass::DEPTH;
            }

            ImGui::SameLine();
            isSelected = *drawPass == game_RenderPass::SHADOW;
            if (ImGui::Selectable("Shadow", &isSelected, 0, ImVec2(100, 20))) {
                *drawPass = game_RenderPass::SHADOW;
            }

            ImGui::SameLine();
            isSelected = *drawPass == game_RenderPass::LIGHTING;
            if (ImGui::Selectable("Lighting", &isSelected, 0, ImVec2(100, 20))) {
                *drawPass = game_RenderPass::LIGHTING;
            }
            ImGui::EndChild();
        }

        // Draw play bar
        {
            ImGui::SameLine();
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
    }

    void
    edit_EditView::DrawOnGUI(game_Entity* selectedEntity)
    {
        m_editCamera.UpdateFPS();
        const math_Mat4x4 view = m_editCamera.GetView();
        const math_Mat4x4 proj = m_editCamera.GetProjection();

        // TODO: Actual Play/Edit mode toggle functionality
        //    probably through edit_Editor
        bool m_isPlaying = false;
        DrawGameViewMenu(&m_isPlaying, &m_drawGrid, &m_drawGizmos, &m_drawPass);
        m_gizmoView.UpdateAndDraw(m_drawGrid, m_drawGizmos, *selectedEntity, m_viewPos, m_viewSize, view, proj);

        m_isHovered = ImGui::IsWindowHovered();

        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 curs   = ImGui::GetCursorPos(); // The start of the game view (after menu)
        m_viewPos     = math_Vec2(winPos.x + curs.x, winPos.y + curs.y);

        ImVec2 winSize = ImGui::GetWindowSize();
        winSize.y -= curs.y;
        const float aspect = winSize.y / winSize.x;
        const float margin = 15.0f;
        ImVec2      gameWinSize(winSize.x - margin, (winSize.x - margin) * aspect);
        m_viewSize = math_Vec2(gameWinSize.x, gameWinSize.y);

        const gfx_RenderTarget* prevRt = gfx_RenderTarget_GetActiveRTV();

        // Draw world to texture
        gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
        m_rtGameMs.Bind();
        m_rtGameMs.Clear();
        m_world->Draw(view, proj, m_drawPass);

        // Redraw to non-multisampling texture (for ImGui)
        gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
        m_rtGame.Bind();
        m_rtGame.Clear();
        m_world->GetRenderer()->DrawRenderToView(&m_rtGameMs, m_multisampleEffect);

        if (prevRt == nullptr) {
            gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
        } else {
            prevRt->Bind();
        }
        ImGui::Image(m_rtGame.GetNativeTexture(), gameWinSize);

        // Right mouse click to open entity context menu
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            m_cursorDown = math_Vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            math_Vec2 cursorUp = math_Vec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
            if (math_LengthSquared(cursorUp - m_cursorDown) <= CLICK_RADIUS * CLICK_RADIUS) {
                m_showEntityContext = EntityAtCursor() == *selectedEntity;
            } else {
                m_showEntityContext = false;
            }
        }

        // Entity context menu
        if (m_showEntityContext) {
            std::stringstream ss;
            ss << "GameEntityContextMenu" << selectedEntity->id;
            if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                if (ImGui::Selectable("Delete entity")) {
                    m_cstack->Do(edit_CommandDeleteEntity::Create(*selectedEntity, m_world));
                    *selectedEntity = game_EntityId_Invalid;
                }
                ImGui::EndPopup();
            }
        }


        // Left mouse click to (de-)select entity
        if (m_isHovered) {
            if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
                if (!m_gizmoView.IsActive()) {
                    *selectedEntity = EntityAtCursor();
                }
            }
        }
    }

    const bool
    edit_EditView::IsHovered() const
    {
        return m_isHovered;
    }


    // ========================================
    // edit_EntityHierarchyView
    // ========================================
    static void
    ApplyDrop(game_World* world, edit_CommandStack* cstack, const game_Entity& targetEntity)
    {
        const game_Entity& droppedEntity = *(game_Entity*)ImGui::GetDragDropPayload()->Data;
        game_TransformId   srcTransform  = world->GetTransformManager()->GetTransformId(droppedEntity);
        game_TransformId   destTransform
            = targetEntity == game_EntityId_Invalid ? game_TransformId_Invalid : world->GetTransformManager()->GetTransformId(targetEntity);
        if (destTransform != game_TransformId_Invalid && world->GetTransformManager()->IsAncestor(destTransform, srcTransform)) {
            // No cycles allowed.
            return;
        }
        cstack->Do(edit_CommandSetParent::Create(srcTransform, destTransform, world->GetTransformManager()));
    }

    static void
    DestroyRecursive(game_World* world, const game_Entity& entity, std::vector<game_Entity>* entitiesRemove)
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
                                             edit_CommandStack*        cstack,
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
                ApplyDrop(world, cstack, entity);
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
                                            edit_CommandStack*        cstack,
                                            const game_Entity&        entity,
                                            game_Entity*              selectedEntity,
                                            std::vector<game_Entity>* entitiesRemove)
    {
        const game_EntityManager* emanager = world->GetEntityManager();
        game_TransformManager*    tmanager = world->GetTransformManager();
        auto                      tid      = tmanager->GetTransformId(entity);
        auto                      child    = tmanager->GetFirstChild(tid);
        if (child == game_TransformId_Invalid) {
            DrawEntityNode(world, cstack, entity, selectedEntity, entitiesRemove, true);
        } else {
            bool isOpen = DrawEntityNode(world, cstack, entity, selectedEntity, entitiesRemove, false);
            if (isOpen) {
                while (child != game_TransformId_Invalid) {
                    game_Entity childEnt = tmanager->GetEntity(child);
                    if (emanager->IsEntityAlive(childEnt)) {
                        DrawLocalTree(world, cstack, childEnt, selectedEntity, entitiesRemove);
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
                    DrawLocalTree(world, cstack, entity, selectedEntity, &entitiesRemove);
                }
            } else {
                DrawEntityNode(world, cstack, entity, selectedEntity, &entitiesRemove, true);
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
                        ApplyDrop(world, cstack, game_EntityId_Invalid);
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


    // ========================================
    // edit_ResourceView
    // ========================================
    void*
    edit_ResourceView::RenderMeshPreviewTexture(const res_Mesh* mesh, const res_Material* material)
    {
        game_DirectionalLight light;
        light.entity    = game_EntityId_Invalid;
        light.color     = math_Vec3::One();
        light.strength  = 1;
        light.direction = math_Vec3(-1, 0, -1);

        const float       meshSize   = math_Length(mesh->GetAABB().max - mesh->GetAABB().min);
        const math_Mat4x4 cameraView = math_LookAt(math_Vec3(1, 1, 1.5f) * meshSize, math_Vec3::Zero());
        const math_Mat4x4 cameraProj = math_PerspectiveFovRH(math_DegToRad(60.0f), PREVIEW_RESOLUTION.x / PREVIEW_RESOLUTION.y, 0.01f, 100.0f);

        m_previewRenderer.SetCamera(cameraView, cameraProj);
        m_previewRenderer.SetDirectionalLight(0, light);

        m_previewRT.Bind();
        m_previewRT.Clear();
        m_previewRenderer.DrawMesh(mesh, material, math_Mat4x4::Identity(), game_RenderPass::LIGHTING);

        gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);

        return m_previewRT.GetNativeTexture();
    }

    edit_ResourceView::edit_ResourceView(gfx_GraphicsAdapter* graphicsAdapter,
                                         gfx_GraphicsDevice*  graphicsDevice,
                                         game_World*          world,
                                         res_ResourceManager* resources)
        : m_graphicsAdapter(graphicsAdapter)
        , m_world(world)
        , m_resources(resources)
        , m_previewRT(graphicsAdapter, PREVIEW_RESOLUTION.x, PREVIEW_RESOLUTION.y, true, false)
        , m_previewRenderer(graphicsAdapter, graphicsDevice, resources)
    {}

    void
    edit_ResourceView::DrawOnGUI(const game_Entity& selectedEntity, edit_CommandStack* cstack)
    {
        if (m_currentDir != ROOT_DIR) {
            if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
                m_currentDir.pop_back();
                while (m_currentDir[m_currentDir.size() - 1] != '\\') {
                    m_currentDir.pop_back();
                }
                m_selectedFile.clear();
            }
        }

        bool showMeshes = m_filterMask & FILTER_FLAG_MESH;
        ImGui::SameLine();
        if (ImGui::Checkbox(".mesh", &showMeshes)) {
            m_filterMask ^= FILTER_FLAG_MESH;
        }
        ImGui::SameLine();

        bool showMats = m_filterMask & FILTER_FLAG_MAT;
        if (ImGui::Checkbox(".mat", &showMats)) {
            m_filterMask ^= FILTER_FLAG_MAT;
        }

        if (!m_selectedFile.empty()) {
            std::string ext = core_GetExtensionFromPath(m_selectedFile.c_str());

            if (ext == "mesh") {
                std::string meshPath;
                {
                    std::stringstream ss;
                    ss << m_currentDir << "\\" << m_selectedFile;
                    meshPath = ss.str();
                }

                if (selectedEntity != game_EntityId_Invalid) {
                    ImGui::SameLine();
                    if (ImGui::Button("Set mesh")) {
                        auto mid = m_world->GetMeshManager()->GetMeshId(selectedEntity);
                        m_world->GetMeshManager()->SetMesh(mid, m_resources->GetMesh(meshPath.c_str()));
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("New mesh entity")) {
                    auto createCommand = edit_CommandCreateEntity::Create(m_world);
                    createCommand->Do();
                    game_Entity newmesh = ((edit_CommandCreateEntity*)createCommand.get())->GetCreatedEntity();
                    cstack->Add(std::move(createCommand));

                    m_world->GetTransformManager()->CreateTransform(newmesh);
                    auto mid = m_world->GetMeshManager()->CreateMesh(newmesh);
                    m_world->GetMeshManager()->SetMesh(mid, m_resources->GetMesh(meshPath.c_str()));
                    m_world->GetMeshManager()->SetMaterial(mid, m_resources->GetMaterial("data\\Dungeon Pack Export\\DungeonPack.mat"));

                    std::string meshname;
                    {
                        std::stringstream ss;
                        auto              fn = core_GetFilenameFromPath(m_selectedFile.c_str());
                        ss << core_GetFilenameFromPath(m_selectedFile.c_str());
                        ss << " [" << std::to_string(newmesh.id) << "]";
                        meshname = ss.str();
                    }
                    m_world->GetEntityManager()->SetName(newmesh, meshname.c_str());
                }
            }
        }


        ImGui::BeginChild("ResourceTree", ImVec2(0, 0), true);
        auto dirItems = core_FSItemsInDirectory(m_currentDir.c_str());
        for (const auto& dirItem : dirItems) {
            switch (dirItem.type) {
                case core_FSItemType::FILE: {
                    std::string filename = core_GetFilenameFromPath(dirItem.path.c_str());
                    std::string ext      = core_GetExtensionFromPath(filename.c_str());
                    if ((ext != "mesh" || showMeshes) && (ext != "mat" || showMats)) {
                        std::stringstream ss;
                        ss << ICON_FA_FILE << " " << filename;
                        if (ImGui::Selectable(ss.str().c_str(), m_selectedFile == filename)) {
                            m_selectedFile = filename;
                        }
                    }

                } break;
                case core_FSItemType::DIRECTORY: {
                    std::stringstream ss;
                    ss << ICON_FA_FOLDER << " " << core_GetDirnameFromPath(dirItem.path.c_str());
                    if (ImGui::Selectable(ss.str().c_str())) {
                        m_currentDir = dirItem.path;
                    }
                } break;
            }
        }
        ImGui::EndChild();
    }

    void
    edit_ResourceView::DrawPreviewOnGUI()
    {
        if (!m_selectedFile.empty() && core_GetExtensionFromPath(m_selectedFile.c_str()) == "mesh") {
            ImVec2    prevWinSize = ImGui::GetWindowSize();
            math_Vec2 previewMargin(10, 40);
            math_Vec2 previewSize(prevWinSize.x - previewMargin.x, prevWinSize.y - previewMargin.y);

            std::string meshPath;
            {
                std::stringstream ss;
                ss << m_currentDir << "\\" << m_selectedFile;
                meshPath = ss.str();
            }
            ImGui::Image(RenderMeshPreviewTexture(m_resources->GetMesh(meshPath.c_str()),
                                                  m_resources->GetMaterial("data\\Dungeon Pack Export\\DungeonPack.mat")),
                         ImVec2(previewSize.x, previewSize.y));
        } else {
            ImGui::Text("Select a file to preview it here");
        }
    }


    // ========================================
    // edit_InspectorView
    // ========================================
    edit_InspectorView::edit_InspectorView(game_World* world, gfx_GraphicsAdapter* graphicsAdapter, res_ResourceManager* resources)
        : m_world(world)
        , m_nameEditor(world->GetEntityManager())
        , m_transformEditor(world->GetTransformManager())
        , m_lightEditor(world, graphicsAdapter)
        , m_cameraEditor(world, graphicsAdapter)
    {
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_MeshEditor(world->GetMeshManager(), resources)));
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_ScriptEditor(world->GetScriptManager())));
    }

    void
    edit_InspectorView::DrawOnGUI(const game_Entity& selectedEntity)
    {
        if (selectedEntity.id != game_EntityId_Invalid) {
            m_nameEditor.UpdateAndDraw(selectedEntity);
            m_transformEditor.UpdateAndDraw(selectedEntity);
            if (m_world->GetLightManager()->HasLight(selectedEntity)) {
                m_lightEditor.UpdateAndDraw(selectedEntity);
            } else if (m_world->GetCameraManager()->HasCamera(selectedEntity)) {
                m_cameraEditor.UpdateAndDraw(selectedEntity);
            } else {
                for (auto& compEditor : m_componentEditors) {
                    compEditor->UpdateAndDraw(selectedEntity);
                }
            }
        }
    }
} // namespace pge