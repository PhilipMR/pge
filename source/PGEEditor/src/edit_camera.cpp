#include "../include/edit_camera.h"

#include <core_assert.h>
#include <imgui/imgui.h>
#include <sstream>
#include <gfx_debug_draw.h>

namespace pge
{
    // =========================================
    // edit_CameraEditor
    // =========================================
    edit_CameraEditor::edit_CameraEditor(game_World* world, gfx_GraphicsAdapter* graphicsAdapter)
        : m_world(world)
        , m_graphicsAdapter(graphicsAdapter)
        , m_camPreviewRT(graphicsAdapter, 320, 180, true, false)
    {}

    void
    edit_CameraEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Camera"))
            return;

        game_CameraManager* camManager = m_world->GetCameraManager();
        core_Assert(camManager->HasCamera(entity));

        struct DragFloatConfig {
            float speed;
            float min;
            float max;
        };
        constexpr DragFloatConfig    DRAG_FOV{1.0f, 0, 0};
        constexpr DragFloatConfig    DRAG_ASPECT{0.1f, 4.0f / 3.0f, 16.0f / 9.0f};
        static const DragFloatConfig DRAG_NEAR_CLIP{0.1f, 0.1f, 10.0f};
        static const DragFloatConfig DRAG_FAR_CLIP{0.1f, 10.0f, 1000.0f};

        game_PerspectiveInfo perspective = camManager->GetPerspective(entity);
        perspective.fov                  = math_RadToDeg(perspective.fov);

        bool changed = false;
        changed |= ImGui::DragFloat("FoV", &perspective.fov, DRAG_FOV.speed, DRAG_FOV.min, DRAG_FOV.max);
        changed |= ImGui::DragFloat("Aspect", &perspective.aspect, DRAG_ASPECT.speed, DRAG_ASPECT.min, DRAG_ASPECT.max);
        changed |= ImGui::DragFloat("Near-clip", &perspective.nearClip, DRAG_NEAR_CLIP.speed, DRAG_NEAR_CLIP.min, DRAG_NEAR_CLIP.max);
        changed |= ImGui::DragFloat("Far-clip", &perspective.farClip, DRAG_FAR_CLIP.speed, DRAG_FAR_CLIP.min, DRAG_FAR_CLIP.max);
        if (changed) {
            perspective.fov = math_DegToRad(perspective.fov);
            camManager->SetPerspective(entity, perspective);
        }


        // Draw render pass selection
        {
            ImGui::BeginChild("Render", ImVec2(200, 40), true);

            bool isSelected = m_renderPass == game_RenderPass::DEPTH;
            if (ImGui::Selectable("Depth", &isSelected, 0, ImVec2(80, 20))) {
                m_renderPass = game_RenderPass::DEPTH;
            }

            ImGui::SameLine();
            isSelected = m_renderPass == game_RenderPass::LIGHTING;
            if (ImGui::Selectable("Lighting", &isSelected, 0, ImVec2(100, 20))) {
                m_renderPass = game_RenderPass::LIGHTING;
            }
            ImGui::EndChild();
        }

        // Push render target
        const auto* prevRT = gfx_RenderTarget_GetActiveRTV();
        m_camPreviewRT.Bind();
        m_camPreviewRT.Clear();

        const math_Mat4x4 view = camManager->GetViewMatrix(entity);
        const math_Mat4x4 proj = camManager->GetProjectionMatrix(entity);

        // Draw world
        m_world->Draw(view, proj, m_renderPass);

        // Pop render target
        if (prevRT == nullptr) {
            gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
        } else {
            prevRT->Bind();
        }

        auto previewTex = reinterpret_cast<ImTextureID>(m_camPreviewRT.GetNativeTexture());
        ImGui::Image(previewTex, ImVec2(m_camPreviewRT.GetWidth(), m_camPreviewRT.GetHeight()));
    }


    // =========================================
    // edit_CommandCreateCamera
    // =========================================
    edit_CommandCreateCamera::edit_CommandCreateCamera(game_World* world)
        : m_world(world)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreateCamera::Do()
    {
        if (m_createdEntity != game_EntityId_Invalid) {
            core_Assert(!m_world->GetEntityManager()->IsEntityAlive(m_createdEntity));
            m_world->GetEntityManager()->CreateEntity(m_createdEntity);
            m_world->InsertSerializedEntity(m_sentity, m_createdEntity);
        } else {
            m_createdEntity = m_world->GetEntityManager()->CreateEntity();
            std::stringstream ss;
            ss << "Camera [" << m_createdEntity.id << "]";
            m_world->GetEntityManager()->SetName(m_createdEntity, ss.str().c_str());
            m_world->GetCameraManager()->CreateCamera(m_createdEntity);
        }
    }

    void
    edit_CommandCreateCamera::Undo()
    {
        core_Assert(m_createdEntity != game_EntityId_Invalid);
        m_sentity = m_world->SerializeEntity(m_createdEntity);
        m_world->GetEntityManager()->DestroyEntity(m_createdEntity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateCamera::Create(game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateCamera(world));
    }
} // namespace pge