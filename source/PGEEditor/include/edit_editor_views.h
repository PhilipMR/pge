#ifndef PGE_EDITOR_EDIT_EDITOR_VIEWS_H
#define PGE_EDITOR_EDIT_EDITOR_VIEWS_H

#include "edit_command.h"
#include <gfx_render_target.h>
#include <gfx_debug_draw.h>
#include <input_mouse.h>
#include <game_world.h>
#include <imgui/IconFontAwesome5.h>
#include <imgui/imgui.h>

namespace pge
{
    static const char* PATH_TO_LAYOUT_INI = "layout.ini";

    inline void
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

    inline void
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

    class edit_GameView {
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_RenderTarget     m_rtGameMs;
        gfx_RenderTarget     m_rtGame;
        const res_Effect*    m_multisampleEffect;

    public:
        edit_GameView(gfx_GraphicsAdapter* graphicsAdapter, res_ResourceManager* resources, unsigned width, unsigned height)
            : m_graphicsAdapter(graphicsAdapter)
            , m_rtGameMs(graphicsAdapter, width, height, true, true)
            , m_rtGame(graphicsAdapter, width, height, false, false)
            , m_multisampleEffect(resources->GetEffect("data\\effects\\multisample.effect"))
        {}

        void
        DrawOnGUI(game_World* world, const math_Vec2& size)
        {
            const gfx_RenderTarget* prevRt = gfx_RenderTarget_GetActiveRTV();

            // Draw world to texture
            gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
            m_rtGameMs.Bind();
            m_rtGameMs.Clear();
            world->Draw();
            gfx_DebugDraw_Flush();

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
    };

} // namespace pge

#endif