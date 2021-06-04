#include "../include/edit_camera.h"

#include <core_assert.h>
#include <imgui/imgui.h>

namespace pge
{
    edit_CameraEditor::edit_CameraEditor(game_CameraManager* cm, gfx_GraphicsAdapter* graphicsAdapter, game_World* world)
        : m_cameraManager(cm)
        , m_graphicsAdapter(graphicsAdapter)
        , m_camPreviewRT(graphicsAdapter, 320, 180, true, false)
        , m_world(world)
    {}

    void
    edit_CameraEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Camera"))
            return;

        core_Assert(m_cameraManager->HasCamera(entity));
        float fov, aspect, nearClip, farClip;
        m_cameraManager->GetPerspectiveFov(entity, &fov, &aspect, &nearClip, &farClip);
        fov          = math_RadToDeg(fov);
        bool changed = false;
        changed |= ImGui::DragFloat("FoV", &fov, 1.0f, 60.0f, 120.0f);
        changed |= ImGui::DragFloat("Aspect", &aspect, 0.1f, (4.0f / 3.0f), (16.0f / 9.0f));
        changed |= ImGui::DragFloat("Near-clip", &nearClip, 0.01f, 0.01f, farClip);
        changed |= ImGui::DragFloat("Far-clip", &farClip, 0.01f, nearClip);
        if (changed) {
            m_cameraManager->SetPerspectiveFov(entity, math_DegToRad(fov), aspect, nearClip, farClip);
        }

        // Push render target
        const auto* prevRT = gfx_RenderTarget_GetActiveRTV();
        m_camPreviewRT.Bind();
        m_camPreviewRT.Clear();

        // Push camera
        const game_Entity prevCamera = m_cameraManager->GetActiveCamera();
        m_cameraManager->Activate(entity);

        // Draw world
        m_world->Draw();

        // Pop camera
        m_cameraManager->Activate(prevCamera);

        // Pop render target
        if (prevRT == nullptr) {
             gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
        } else {
            prevRT->Bind();
        }

        auto previewTex = reinterpret_cast<ImTextureID>(m_camPreviewRT.GetNativeTexture());
        ImGui::Image(previewTex, ImVec2(m_camPreviewRT.GetWidth(), m_camPreviewRT.GetHeight()));
    }
} // namespace pge