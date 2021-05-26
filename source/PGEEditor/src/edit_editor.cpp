#include "../include/edit_editor.h"
#include "../include/edit_mesh.h"
#include "../include/edit_light.h"
#include "../include/edit_script.h"
#include <anim_skeleton.h>
#include <input_mouse.h>
#include <input_keyboard.h>
#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <game_world.h>
#include <imgui/IconFontAwesome5.h>
#include <imgui/ImGuizmo.h>
#include <sstream>
#include <time.h>

#include <anim_animator.h>

namespace pge
{
    extern void                   edit_BeginFrame();
    extern void                   edit_EndFrame();
    static const char*            PATH_TO_LAYOUT_INI = "layout.ini";
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
        , m_previewRT(graphicsAdapter, PREVIEW_RESOLUTION.x, PREVIEW_RESOLUTION.y, true, false)
    {
        ImGui::LoadIniSettingsFromDisk(PATH_TO_LAYOUT_INI);
        m_componentEditors.push_back(std::unique_ptr<edit_ComponentEditor>(new edit_TransformEditor(m_world->GetTransformManager())));
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
    edit_Editor::UpdateAndDraw(const gfx_RenderTarget* target)
    {
        edit_BeginFrame();

        HandleEvents();

        if (m_editMode == EDITOR_MODE_EDIT) {
            DrawMenuBar();
        }

        m_world->GetAnimationManager()->Update(1.0f / 60.0f);
        if (m_editMode == EDITOR_MODE_PLAY) {
//            m_world->Update();
            m_world->GetBehaviourManager()->Update(1.0f / 60.0f);
        } else {
            game_Camera_UpdateFPS(m_world->GetCamera(), .1f);
        }
        bool ishovering = DrawGameView(target);

        if (m_editMode == EDITOR_MODE_EDIT) {
            DrawGizmos();
            DrawEntityTree();
            DrawInspector();
            DrawLog();
            DrawResources();
        }

        edit_EndFrame();
        return ishovering;
    }


    game_Entity
    edit_Editor::SelectEntity() const
    {
        auto*             scene    = m_world.get();
        const math_Mat4x4 viewProj = scene->GetCamera()->GetProjectionMatrix() * scene->GetCamera()->GetViewMatrix();

        // Static Mesh select
        const math_Ray ray = math_Raycast_RayFromPixel(input_MousePosition() - m_gameWindowPos, m_gameWindowSize, viewProj);
        float          meshSelectDistance;
        game_Entity    meshSelectEntity
            = scene->GetStaticMeshManager()->RaycastSelect(*scene->GetTransformManager(), ray, viewProj, &meshSelectDistance);


        // Point Light select
        math_Vec2 hoverPosNorm(input_MousePosition());
        hoverPosNorm.x -= m_gameWindowPos.x;
        hoverPosNorm.y -= m_gameWindowPos.y;
        hoverPosNorm.x /= m_gameWindowSize.x;
        hoverPosNorm.y /= m_gameWindowSize.y;

        math_Vec2   billboardSize(2, 2);
        float       lightSelectDistance;
        game_Entity lightSelectEntity = scene->GetLightManager()->HoverSelect(*m_world->GetTransformManager(),
                                                                              hoverPosNorm,
                                                                              billboardSize,
                                                                              scene->GetCamera()->GetViewMatrix(),
                                                                              scene->GetCamera()->GetProjectionMatrix(),
                                                                              &lightSelectDistance);


        // Choose the closest one
        game_Entity whichOne[]     = {game_EntityId_Invalid,
                                  meshSelectEntity,
                                  lightSelectEntity,
                                  meshSelectDistance <= lightSelectDistance ? meshSelectEntity : lightSelectEntity};
        unsigned    selectMeshBit  = unsigned(meshSelectEntity != game_EntityId_Invalid);
        unsigned    selectLightBit = unsigned(lightSelectEntity != game_EntityId_Invalid) << 1;
        return whichOne[selectMeshBit | selectLightBit];
    }

    void
    edit_Editor::DrawGizmos()
    {
        auto* scene = m_world.get();

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
            m_transformGizmo.TransformEntity(m_selectedEntity, m_world->GetCamera()->GetViewMatrix(), m_world->GetCamera()->GetProjectionMatrix());

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

                bool isDirLight   = scene->GetLightManager()->HasDirectionalLight(entity);
                bool isPointLight = scene->GetLightManager()->HasPointLight(entity);
                core_Assert(!(isDirLight && isPointLight));
                if (isDirLight || isPointLight) {
                    math_Vec3 plightPos;
                    if (scene->GetTransformManager()->HasTransform(entity)) {
                        auto tid = scene->GetTransformManager()->GetTransformId(entity);
                        plightPos += scene->GetTransformManager()->GetWorldPosition(tid);
                    }
                    gfx_DebugDraw_Billboard(plightPos, math_Vec2(2, 2), m_icons.pointLight);
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

        // Left mouse click to (de-)select entity
        if (input_MouseButtonPressed(input_MouseButton::LEFT)) {
            if (!ImGuizmo::IsOver() || !m_transformGizmo.IsVisible()) {
                m_selectedEntity = SelectEntity();
            }
        }
    }

    void
    edit_Editor::DrawMenuBar()
    {
        auto* scene = m_world.get();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New scene", "CTRL+N")) {}
                if (ImGui::MenuItem("Save scene", "CTRL+S")) {
                    std::ofstream os("test.world");
                    os << *scene;
                    os.close();
                }
                if (ImGui::MenuItem("Save scene as...", "CTRL+SHIFT+S")) {}
                if (ImGui::MenuItem("Load scene...", "CTRL+L")) {
                    std::ifstream is("test.world");
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
        auto* scene = m_world.get();


        ImGui::Begin("Game", nullptr, PANEL_WINDOW_FLAGS);

        float barWidth = ImGui::GetContentRegionAvailWidth();
        if (m_editMode == EDITOR_MODE_EDIT) {
            ImGui::Checkbox("Grid", &m_drawGrid);
            ImGui::SameLine();
            ImGui::Checkbox("Gizmos", &m_drawGizmos);
            ImGui::SameLine();
        }

        static bool  isPlaying = false;
        const ImVec2 size(50, 20);
        ImGui::SetCursorPosX(barWidth - size.x);
        if (!isPlaying) {
            if (ImGui::Button(ICON_FA_PLAY, size)) {
                m_editMode = EDITOR_MODE_PLAY;
                isPlaying  = true;
            }
        } else {
            if (ImGui::Button(ICON_FA_PAUSE, size)) {
                m_editMode = EDITOR_MODE_EDIT;
                isPlaying  = false;
            }
        }

        float  r        = 16.0f / 9.0f;
        ImVec2 winPos   = ImGui::GetWindowPos();
        ImVec2 cursPos  = ImGui::GetCursorPos();
        m_gameWindowPos = math_Vec2(winPos.x + cursPos.x, winPos.y + cursPos.y);

        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 gameWinSize(winSize.x - 20, (winSize.y - (cursPos.y + 5)) - 20 * r);
        m_gameWindowSize = math_Vec2(gameWinSize.x, gameWinSize.y);

        ImGui::Image(target->GetNativeTexture(), gameWinSize);
        bool isHovered = ImGui::IsWindowHovered();

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
        auto* world = m_world.get();

        ImGui::Begin("World graph", nullptr, PANEL_WINDOW_FLAGS);

        // ImGui::Image(m_icons.sceneNode, ImVec2(15, 15));
        ImGui::SameLine();
        ImGui::Text("%s World", ICON_FA_CARET_SQUARE_DOWN);
        ImGui::Indent();

        bool                     isAnyNodeHovered    = false;
        bool                     isEntityContextMenu = false;
        auto*                    mm                  = world->GetEntityMetaDataManager();
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
                std::stringstream textIdSs;
                textIdSs << "##editname" << entity.entity.id;
                std::string textIdStr = textIdSs.str();
                if (ImGui::InputText(textIdStr.c_str(), (char*)entity.name, sizeof(entity.name), ImGuiInputTextFlags_EnterReturnsTrue)) {
                    m_selectedEntity = entity.entity;
                    editEntityId     = game_EntityId_Invalid;
                }
            } else {
                ImGui::Selectable(entity.name, isSelected);
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    editEntityId = entity.entity.id;
                } else if (ImGui::IsItemClicked()) {
                    if (m_selectedEntity.id != entity.entity.id) {
                        m_selectedEntity = entity.entity;
                        editEntityId     = game_EntityId_Invalid;
                    }
                }

                std::stringstream ss;
                ss << "WorldGraphEntityContextMenu" << entity.entity.id;
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
            m_commandStack.Do(edit_CommandDeleteEntity::Create(entity, world));
            if (m_selectedEntity == entity) {
                m_selectedEntity = game_EntityId_Invalid;
            }
        }

        if (!isAnyNodeHovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_selectedEntity = game_EntityId_Invalid;
        }

        if (!isEntityContextMenu && ImGui::BeginPopupContextWindow("WorldContextMenu")) {
            if (ImGui::Selectable("Create entity")) {
                m_commandStack.Do(edit_CommandCreateEntity::Create(world));
            }
            if (ImGui::Selectable("Create light (directional)")) {
                m_commandStack.Do(edit_CommandCreateDirectionalLight::Create(world));
            }
            if (ImGui::Selectable("Create light (point)")) {
                m_commandStack.Do(edit_CommandCreatePointLight::Create(world));
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
            if (m_world->GetLightManager()->HasDirectionalLight(m_selectedEntity) || m_world->GetLightManager()->HasPointLight(m_selectedEntity)) {
                edit_TransformEditor(m_world->GetTransformManager()).UpdateAndDraw(m_selectedEntity);
                edit_LightEditor(m_world->GetLightManager()).UpdateAndDraw(m_selectedEntity);
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
        auto               dirItems   = core_FSItemsInDirectory(currentDir.c_str());
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
                    m_world->GetEntityMetaDataManager()->SetMetaData(newmesh, game_EntityMetaData(newmesh, meshname.c_str()));
                }
            }
        }

        ImGui::BeginChild("ResourceTree", ImVec2(0, 0), true);
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

        game_Camera camera;
        camera.SetPerspectiveFov(math_DegToRad(60.0f), PREVIEW_RESOLUTION.x / PREVIEW_RESOLUTION.y, 0.01f, 100.0f);
        const float meshSize = math_Length(mesh->GetAABB().max - mesh->GetAABB().min);
        camera.SetLookAt(math_Vec3(1, 1, 1.5f) * meshSize, math_Vec3::Zero());

        game_Renderer renderer(m_graphicsAdapter, m_graphicsDevice);
        renderer.SetCamera(&camera);
        renderer.SetDirectionalLight(0, light);

        m_previewRT.Bind();
        m_previewRT.Clear();
        renderer.DrawMesh(mesh, material, math_Mat4x4::Identity());

        gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);

        return reinterpret_cast<ImTextureID>(m_previewRT.GetNativeTexture());
    }
} // namespace pge
