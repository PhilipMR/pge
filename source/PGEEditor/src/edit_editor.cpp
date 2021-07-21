#include "../include/edit_editor.h"

#include "../include/edit_entity.h"
#include <gfx_debug_draw.h>
#include <input_keyboard.h>
#include <game_world.h>
#include <imgui/imgui.h>
#include <time.h>

namespace pge
{
    extern void edit_BeginFrame();
    extern void edit_EndFrame();

    edit_Editor::edit_Editor(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_graphicsAdapter(graphicsAdapter)
        , m_graphicsDevice(graphicsDevice)
        , m_resources(resources)
        , m_world(std::make_unique<game_World>(m_graphicsAdapter, m_graphicsDevice, m_resources))
        , m_editView(graphicsAdapter, resources, m_world.get(), &m_commandStack, 1600, 900)
        , m_resourceView(graphicsAdapter, graphicsDevice, m_world.get(), m_resources)
        , m_inspectorView(m_world.get(), m_graphicsAdapter, m_resources)
    {
        ImGui::LoadIniSettingsFromDisk(edit_PATH_TO_LAYOUT_INI);
    }

    bool
    edit_Editor::UpdateAndDraw()
    {
        gfx_RenderTarget_ClearMainRTV(m_graphicsAdapter);
        m_world->GarbageCollect();

        edit_BeginFrame();
        gfx_DebugDraw_Clear();

        HandleShortcuts();
        m_world->GetAnimationManager()->Update(1.0f / 60.0f);

        edit_DrawMainMenuBar(m_world.get(), &m_commandStack);

        const ImGuiWindowFlags PANEL_WINDOW_FLAGS = ImGuiWindowFlags_NoTitleBar;

        ImGui::Begin("Log", nullptr, PANEL_WINDOW_FLAGS);
        m_logView.DrawOnGUI();
        ImGui::End();

        ImGui::Begin("World graph", nullptr, PANEL_WINDOW_FLAGS);
        m_hierarchyView.DrawOnGUI(m_world.get(), &m_selectedEntity, &m_commandStack);
        ImGui::End();

        ImGui::Begin("Resources", nullptr, PANEL_WINDOW_FLAGS);
        m_resourceView.DrawOnGUI(m_selectedEntity, &m_commandStack);
        ImGui::End();

        ImGui::Begin("Preview");
        m_resourceView.DrawPreviewOnGUI();
        ImGui::End();

        ImGui::Begin("Game", nullptr, PANEL_WINDOW_FLAGS);
        m_editView.DrawOnGUI(&m_selectedEntity);
        ImGui::End();

        ImGui::Begin("Inspector", nullptr, PANEL_WINDOW_FLAGS);
        m_inspectorView.DrawOnGUI(m_selectedEntity);
        ImGui::End();

        edit_EndFrame();

        return m_editView.IsHovered();
    }

    void
    edit_Editor::HandleShortcuts()
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

} // namespace pge
