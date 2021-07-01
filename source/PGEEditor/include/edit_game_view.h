#ifndef PGE_EDITOR_EDIT_GAME_VIEW_H
#define PGE_EDITOR_EDIT_GAME_VIEW_H

#include <gfx_render_target.h>
#include <res_resource_manager.h>
#include <imgui/imgui.h>
#include <game_world.h>
#include <gfx_debug_draw.h>

namespace pge
{
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