#include "../include/edit_editor.h"

#include "../include/edit_mesh.h"
#include "../include/edit_script.h"

#include <anim_skeleton.h>
#include <input_mouse.h>
#include <input_keyboard.h>
#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <game_world.h>
#include <imgui/ImGuizmo.h>
#include <sstream>
#include <time.h>

namespace pge
{
    extern void                   edit_BeginFrame();
    extern void                   edit_EndFrame();
    static const ImGuiWindowFlags PANEL_WINDOW_FLAGS = ImGuiWindowFlags_NoTitleBar;


    edit_Editor::edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_editMode(EDITOR_MODE_EDIT)
        , m_graphicsAdapter(graphicsAdapter)
        , m_graphicsDevice(graphicsDevice)
        , m_resources(resources)
        , m_world(std::make_unique<game_World>(m_graphicsAdapter, m_graphicsDevice, m_resources))
        , m_transformGizmo(m_world->GetTransformManager(), &m_commandStack)
        , m_selectedEntity(game_EntityId_Invalid)
        , m_drawGrid(true)
        , m_drawGizmos(true)
        , m_gameWindowSize(1600, 900)
        , m_lightEditor(m_world->GetLightManager())
        , m_cameraEditor(m_world->GetCameraManager(), m_graphicsAdapter, m_world.get())
        , m_previewRT(graphicsAdapter, PREVIEW_RESOLUTION.x, PREVIEW_RESOLUTION.y, true, false)
        , m_gameView(graphicsAdapter, resources, 1920, 1080)
    {
        ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_EntityNameEditor(m_world->GetEntityManager())));
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_TransformEditor(m_world->GetTransformManager())));
        m_transformEditor = static_cast<edit_TransformEditor*>(m_componentEditors.back().get());
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_MeshEditor(m_world->GetStaticMeshManager(), resources)));
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_ScriptEditor(m_world->GetScriptManager())));

        auto GetImTexture = [&](const char* path) {
            return (ImTextureID)resources->GetTexture(path)->GetTexture()->GetNativeTexture();
        };
        m_icons.worldNode         = GetImTexture("data/icons/node.png");
        m_icons.worldNodeSelected = GetImTexture("data/icons/node_selected.png");
        m_icons.playButton        = GetImTexture("data/icons/btn_play.png");
        m_icons.pauseButton       = GetImTexture("data/icons/btn_pause.png");
        m_icons.pointLight        = resources->GetTexture("data/icons/light_point.png")->GetTexture();
        m_icons.camera            = resources->GetTexture("data/icons/camera.png")->GetTexture();
    }

    void
    edit_Editor::LoadWorld(const char* path)
    {
        std::ifstream is(path);
        is >> *m_world;
        is.close();
    }

    game_World&
    edit_Editor::GetWorld()
    {
        return *m_world;
    }

    bool
    edit_Editor::UpdateAndDraw()
    {
        gfx_RenderTarget_ClearMainRTV(m_graphicsAdapter);
        m_world->GarbageCollect();

        edit_BeginFrame();
        gfx_DebugDraw_Clear();

        HandleEvents();

        //        ImGui::ShowDemoWindow();

        if (m_editMode == EDITOR_MODE_EDIT) {
            DrawMenuBar();
        }

        m_world->GetAnimationManager()->Update(1.0f / 60.0f);
        if (m_editMode == EDITOR_MODE_PLAY) {
            //            m_world->Update();
            m_world->GetBehaviourManager()->Update(1.0f / 60.0f);
        } else {
            m_editCamera.UpdateFPS();
        }
        DrawLog();
        if (m_editMode == EDITOR_MODE_EDIT) {
            m_editCamera.Activate();
            DrawGizmos();
            DrawEntityTree();
            DrawResources();
        }
        bool ishovering = DrawGameView();
        if (m_editMode == EDITOR_MODE_EDIT) {
            DrawInspector();
        }

        edit_EndFrame();
        return ishovering;
    }


    game_Entity
    edit_Editor::EntityAtCursor() const
    {
        math_Vec2 cursor = input_MousePosition() - m_gameWindowPos;
        return m_world->FindEntityAtCursor(cursor, m_gameWindowSize, m_editCamera.GetView(), m_editCamera.GetProjection());
    }

    void
    edit_Editor::DrawGizmos()
    {
        if (m_drawGrid) {
            const float     axisThickness = 0.1f;
            const float     thickness     = 0.025f;
            const float     length        = 1000.0f;
            const math_Vec2 cellSize(5.0f, 5.0f);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(length, 0, 0), math_Vec3(1, 0, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, length, 0), math_Vec3(0, 1, 0), axisThickness);
            gfx_DebugDraw_Line(math_Vec3(0, 0, 0), math_Vec3(0, 0, length), math_Vec3(0, 0, 1), axisThickness);
            gfx_DebugDraw_GridXY(math_Vec3(0, 0, -thickness), length, cellSize, math_Vec3::One(), thickness);
        }

        ImGuizmo::Enable(m_drawGizmos);
        if (m_drawGizmos) {
            ImGuizmo::SetRect(m_gameWindowPos.x, m_gameWindowPos.y, m_gameWindowSize.x, m_gameWindowSize.y);
            m_transformGizmo.TransformEntity(m_selectedEntity, m_editCamera.GetView(), m_editCamera.GetProjection());

            // Selected entity AABB
            if (m_world->GetStaticMeshManager()->HasStaticMesh(m_selectedEntity) && m_world->GetTransformManager()->HasTransform(m_selectedEntity)) {
                auto            meshId = m_world->GetStaticMeshManager()->GetStaticMeshId(m_selectedEntity);
                const res_Mesh* mesh   = m_world->GetStaticMeshManager()->GetMesh(meshId);
                if (mesh != nullptr) {
                    auto transformId = m_world->GetTransformManager()->GetTransformId(m_selectedEntity);
                    auto world       = m_world->GetTransformManager()->GetWorldMatrix(transformId);
                    auto aabb        = mesh->GetAABB();
                    aabb             = math_TransformAABB(aabb, world);
                    gfx_DebugDraw_Box(aabb.min, aabb.max);
                }
            }

            // Light and Camera billboards
            game_EntityManager& emanager = *m_world->GetEntityManager();
            for (const auto& entity : emanager) {
                const bool isDirLight   = m_world->GetLightManager()->HasDirectionalLight(entity);
                const bool isPointLight = m_world->GetLightManager()->HasPointLight(entity);
                core_Assert(!(isDirLight && isPointLight));
                const bool isCamera = m_world->GetCameraManager()->HasCamera(entity);
                if (isDirLight || isPointLight || isCamera) {
                    const auto      tid      = m_world->GetTransformManager()->GetTransformId(entity);
                    const math_Vec3 worldPos = m_world->GetTransformManager()->GetWorldPosition(tid);
                    gfx_DebugDraw_Billboard(worldPos, math_Vec2(2, 2), isCamera ? m_icons.camera : m_icons.pointLight);

                    if (isDirLight) {
                        auto                         lid             = m_world->GetLightManager()->GetDirectionalLightId(entity);
                        const game_DirectionalLight& light           = m_world->GetLightManager()->GetDirectionalLight(lid);
                        constexpr float              DIR_DRAW_LENGTH = 5.0f;
                        const math_Vec3              lightDir
                            = math_Normalize((m_world->GetTransformManager()->GetWorldMatrix(tid) * math_Vec4(light.direction, 0)).xyz);
                        gfx_DebugDraw_Line(worldPos, worldPos + lightDir * DIR_DRAW_LENGTH, light.color);
                    }

                    if (isCamera && entity == m_selectedEntity) {
                        game_PerspectiveInfo perspective = m_world->GetCameraManager()->GetPerspective(entity);
                        const math_Mat4x4    camMatrix   = m_world->GetTransformManager()->GetLocalMatrix(tid);

                        // Draw the frustrum one unit behind the actual camera to prevent it from
                        // obstructing its view.
                        game_DebugDraw_Frustum(perspective, worldPos - camMatrix.Forward(), camMatrix);
                    }
                }
            }
        }
    }

    void
    edit_Editor::HandleEvents()
    {
        static bool doing = false;
        static enum { REDOING, UNDOING } doingMode;
        if (input_KeyboardDown(input_KeyboardKey::CTRL) && input_KeyboardPressed(input_KeyboardKey::Z)) {
            doing     = true;
            doingMode = input_KeyboardDown(input_KeyboardKey::SHIFT) ? REDOING : UNDOING;
        }
        if (input_KeyboardDown(input_KeyboardKey::CTRL) && input_KeyboardPressed(input_KeyboardKey::Y)) {
            doing     = true;
            doingMode = REDOING;
        }
        if (input_KeyboardReleased(input_KeyboardKey::CTRL) || input_KeyboardReleased(input_KeyboardKey::Z)) {
            doing = false;
        }

        static clock_t doTime   = clock();
        clock_t        now      = clock();
        double         downSecs = static_cast<double>(now - doTime) / CLOCKS_PER_SEC;
        if (doing && downSecs >= 0.2) {
            doTime = clock();
            if (doingMode == REDOING) {
                m_commandStack.Redo();
            } else {
                m_commandStack.Undo();
            }
        }


        if (m_selectedEntity != game_EntityId_Invalid && input_KeyboardDown(input_KeyboardKey::CTRL) && input_KeyboardPressed(input_KeyboardKey::D)) {
            auto command = edit_CommandDuplicateEntity::Create(m_world.get(), m_selectedEntity);
            command->Do();
            m_selectedEntity = ((edit_CommandDuplicateEntity*)command.get())->GetCreatedEntity();
            m_commandStack.Add(std::move(command));
        }

    }

    void
    edit_Editor::DrawMenuBar()
    {
        edit_DrawMainMenuBar(m_world.get(), &m_commandStack);
    }

    bool
    edit_Editor::DrawGameView()
    {
        ImGui::Begin("Game", nullptr, PANEL_WINDOW_FLAGS);

        bool isPlaying = m_editMode == EDITOR_MODE_PLAY;
        edit_DrawGameViewMenu(&isPlaying, &m_drawGrid, &m_drawGizmos);
        m_editMode = isPlaying ? EDITOR_MODE_PLAY : EDITOR_MODE_EDIT;

        ImVec2 winPos   = ImGui::GetWindowPos();
        ImVec2 cursPos  = ImGui::GetCursorPos();
        m_gameWindowPos = math_Vec2(winPos.x + cursPos.x, winPos.y + cursPos.y);

        ImVec2 winSize = ImGui::GetWindowSize();
        float  aspect  = m_editCamera.GetAspect();
        ImVec2 gameWinSize(winSize.x - 20, (winSize.y - (cursPos.y + 5)) - 20 * aspect);
        m_gameWindowSize = math_Vec2(gameWinSize.x, gameWinSize.y);

        m_gameView.DrawOnGUI(m_world.get(), m_gameWindowSize, m_editCamera.GetView(), m_editCamera.GetProjection());
        if (ImGui::IsItemHovered())
        {
            // Left mouse click to (de-)select entity
            if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
                if (!ImGuizmo::IsOver() || !m_transformGizmo.IsVisible()) {
                    m_selectedEntity = EntityAtCursor();
                }
            }
        }

        // Right mouse click to open entity context menu
        static bool         hoveringSelectedEntity = false;
        static game_Entity  entDown                = game_EntityId_Invalid;
        static double       downStartTime          = 0;
        static const double deleteClickTime        = 0.5;
        if (input_MouseButtonPressed(input_MouseButton::RIGHT)) {
            entDown       = EntityAtCursor();
            downStartTime = ImGui::GetTime();
        }
        if (input_MouseButtonReleased(input_MouseButton::RIGHT) && (ImGui::GetTime() - downStartTime) <= deleteClickTime) {
            game_Entity selected   = EntityAtCursor();
            hoveringSelectedEntity = (selected.id != game_EntityId_Invalid) && (selected.id == entDown.id) && (selected.id == m_selectedEntity.id);
        }
        if (hoveringSelectedEntity) {
            hoveringSelectedEntity = math_FloatEqual(math_LengthSquared(input_MouseDelta()), 0);
        }

        static bool contextWasOpen = false;
        if (hoveringSelectedEntity) {
            std::stringstream ss;
            ss << "GameEntityContextMenu" << m_selectedEntity.id;
            if (ImGui::BeginPopupContextItem(ss.str().c_str())) {
                contextWasOpen = true;
                if (ImGui::Selectable("Delete entity")) {
                    m_commandStack.Do(edit_CommandDeleteEntity::Create(m_selectedEntity, m_world.get()));
                    m_selectedEntity.id = game_EntityId_Invalid;
                }
                ImGui::EndPopup();
            } else if (contextWasOpen) {
                hoveringSelectedEntity = false;
                contextWasOpen         = false;
            }
        }

        bool isHovered = ImGui::IsWindowHovered();
        ImGui::End();
        return isHovered;
    }

    void
    edit_Editor::DrawEntityTree()
    {
        ImGui::Begin("World graph", nullptr, PANEL_WINDOW_FLAGS);
        m_hierarchyView.DrawOnGUI(m_world.get(), &m_selectedEntity, &m_commandStack);
        ImGui::End();
    }

    void
    edit_Editor::DrawInspector()
    {
        ImGui::Begin("Inspector", nullptr, PANEL_WINDOW_FLAGS);
        if (m_selectedEntity.id != game_EntityId_Invalid) {
            if (m_world->GetLightManager()->HasDirectionalLight(m_selectedEntity) || m_world->GetLightManager()->HasPointLight(m_selectedEntity)) {
                m_transformEditor->UpdateAndDraw(m_selectedEntity);
                m_lightEditor.UpdateAndDraw(m_selectedEntity);
            } else if (m_world->GetCameraManager()->HasCamera(m_selectedEntity)) {
                m_transformEditor->UpdateAndDraw(m_selectedEntity);
                m_cameraEditor.UpdateAndDraw(m_selectedEntity);
            } else {
                for (auto& compEditor : m_componentEditors) {
                    compEditor->UpdateAndDraw(m_selectedEntity);
                }
            }
        }
        ImGui::End();
    }

    void
    edit_Editor::DrawLog()
    {
        ImGui::Begin("Log", nullptr, PANEL_WINDOW_FLAGS);

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

        ImGui::End();
    }

    void
    edit_Editor::DrawResources()
    {
        ImGui::Begin("Resources", nullptr, PANEL_WINDOW_FLAGS);

        static std::string currentDir = "data\\";
        static std::string selectedFile;

        if (currentDir != "data\\") {
            if (ImGui::Button(ICON_FA_ARROW_LEFT)) {
                currentDir.pop_back();
                while (currentDir[currentDir.size() - 1] != '\\') {
                    currentDir.pop_back();
                }
                selectedFile.clear();
            }
        }

        const std::uint8_t  meshFilterFlag = 1 << 0;
        const std::uint8_t  matFilterFlag  = 1 << 1;
        static std::uint8_t filterMask     = meshFilterFlag | matFilterFlag;

        bool showMeshes = filterMask & meshFilterFlag;
        ImGui::SameLine();
        if (ImGui::Checkbox(".mesh", &showMeshes)) {
            filterMask ^= meshFilterFlag;
        }
        ImGui::SameLine();

        bool showMats = filterMask & matFilterFlag;
        if (ImGui::Checkbox(".mat", &showMats)) {
            filterMask ^= matFilterFlag;
        }


        if (!selectedFile.empty()) {
            std::string ext = core_GetExtensionFromPath(selectedFile.c_str());

            if (ext == "mesh") {
                std::string meshPath;
                {
                    std::stringstream ss;
                    ss << currentDir << "\\" << selectedFile;
                    meshPath = ss.str();
                }

                if (m_selectedEntity != game_EntityId_Invalid) {
                    ImGui::SameLine();
                    if (ImGui::Button("Set mesh")) {
                        auto mid = m_world->GetStaticMeshManager()->GetStaticMeshId(m_selectedEntity);
                        m_world->GetStaticMeshManager()->SetMesh(mid, m_resources->GetMesh(meshPath.c_str()));
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("New mesh entity")) {
                    auto createCommand = edit_CommandCreateEntity::Create(m_world.get());
                    createCommand->Do();
                    game_Entity newmesh = ((edit_CommandCreateEntity*)createCommand.get())->GetCreatedEntity();
                    m_commandStack.Add(std::move(createCommand));

                    m_world->GetTransformManager()->CreateTransform(newmesh);
                    auto mid = m_world->GetStaticMeshManager()->CreateStaticMesh(newmesh);
                    m_world->GetStaticMeshManager()->SetMesh(mid, m_resources->GetMesh(meshPath.c_str()));
                    m_world->GetStaticMeshManager()->SetMaterial(mid, m_resources->GetMaterial("data\\Dungeon Pack Export\\DungeonPack.mat"));

                    std::string meshname;
                    {
                        std::stringstream ss;
                        auto              fn = core_GetFilenameFromPath(selectedFile.c_str());
                        ss << core_GetFilenameFromPath(selectedFile.c_str());
                        ss << " [" << std::to_string(newmesh.id) << "]";
                        meshname = ss.str();
                    }
                    m_world->GetEntityManager()->SetName(newmesh, meshname.c_str());
                }
            }
        }

        ImGui::BeginChild("ResourceTree", ImVec2(0, 0), true);
        auto dirItems = core_FSItemsInDirectory(currentDir.c_str());
        for (const auto& dirItem : dirItems) {
            switch (dirItem.type) {
                case core_FSItemType::FILE: {
                    std::string filename = core_GetFilenameFromPath(dirItem.path.c_str());
                    std::string ext      = core_GetExtensionFromPath(filename.c_str());
                    if ((ext != "mesh" || showMeshes) && (ext != "mat" || showMats)) {
                        std::stringstream ss;
                        ss << ICON_FA_FILE << " " << filename;
                        if (ImGui::Selectable(ss.str().c_str(), selectedFile == filename)) {
                            selectedFile = filename;
                        }
                    }

                } break;
                case core_FSItemType::DIRECTORY: {
                    std::stringstream ss;
                    ss << ICON_FA_FOLDER << " " << core_GetDirnameFromPath(dirItem.path.c_str());
                    if (ImGui::Selectable(ss.str().c_str())) {
                        currentDir = dirItem.path;
                    }
                } break;
            }
        }
        ImGui::EndChild();
        ImGui::End();

        ImGui::Begin("Preview");
        if (!selectedFile.empty() && core_GetExtensionFromPath(selectedFile.c_str()) == "mesh") {
            ImVec2    prevWinSize = ImGui::GetWindowSize();
            math_Vec2 previewMargin(10, 40);
            math_Vec2 previewSize(prevWinSize.x - previewMargin.x, prevWinSize.y - previewMargin.y);

            std::string meshPath;
            {
                std::stringstream ss;
                ss << currentDir << "\\" << selectedFile;
                meshPath = ss.str();
            }
            ImGui::Image(RenderMeshPreviewTexture(m_resources->GetMesh(meshPath.c_str()),
                                                  m_resources->GetMaterial("data\\Dungeon Pack Export\\DungeonPack.mat")),
                         ImVec2(previewSize.x, previewSize.y));
        } else {
            ImGui::Text("Select a file to preview it here");
        }

        ImGui::End();
    }

    ImTextureID
    edit_Editor::RenderMeshPreviewTexture(const res_Mesh* mesh, const res_Material* material)
    {
        game_DirectionalLight light;
        light.entity    = game_EntityId_Invalid;
        light.color     = math_Vec3::One();
        light.strength  = 1;
        light.direction = math_Vec3(-1, 0, -1);

        const float       meshSize   = math_Length(mesh->GetAABB().max - mesh->GetAABB().min);
        const math_Mat4x4 cameraView = math_LookAt(math_Vec3(1, 1, 1.5f) * meshSize, math_Vec3::Zero());
        const math_Mat4x4 cameraProj = math_PerspectiveFovRH(math_DegToRad(60.0f), PREVIEW_RESOLUTION.x / PREVIEW_RESOLUTION.y, 0.01f, 100.0f);

        game_Renderer renderer(m_graphicsAdapter, m_graphicsDevice);
        renderer.SetCamera(cameraView, cameraProj);
        renderer.SetDirectionalLight(0, light);

        m_previewRT.Bind();
        m_previewRT.Clear();
        renderer.DrawMesh(mesh, material, math_Mat4x4::Identity());

        gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);

        return reinterpret_cast<ImTextureID>(m_previewRT.GetNativeTexture());
    }
} // namespace pge
