#include "../include/edit_light.h"
#include <imgui/imgui.h>
#include <sstream>

namespace pge
{
    static const unsigned DEPTH_RT_WIDTH = 300;
    static const unsigned DEPTH_RT_HEIGHT = 300;

    edit_LightEditor::edit_LightEditor(game_World* world, gfx_GraphicsAdapter* graphicsAdapter)
        : m_graphicsAdapter(graphicsAdapter)
        , m_world(world)
        , m_depthRT(graphicsAdapter, DEPTH_RT_WIDTH, DEPTH_RT_HEIGHT, true, false)
    {}

    void
    edit_LightEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Light"))
            return;

        game_LightManager* lmanager = m_world->GetLightManager();
        core_Assert(lmanager->HasLight(entity));
        if (lmanager->HasDirectionalLight(entity)) {
            game_DirectionalLightId lid     = lmanager->GetDirectionalLightId(entity);
            game_DirectionalLight   light   = lmanager->GetDirectionalLight(lid);
            bool                    changed = false;
            changed |= ImGui::DragFloat3("Direction", &light.direction[0]);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            changed |= ImGui::DragFloat("Strength", &light.strength);
            if (changed) {
                lmanager->SetDirectionalLight(lid, light);
            }

            // Preview shadow/depth 
            // view, proj:
            math_Mat4x4 view, proj;

            auto tid = m_world->GetTransformManager()->GetTransformId(entity);
            core_Verify(math_Invert(m_world->GetTransformManager()->GetWorldMatrix(tid), &view));

            static float OrthoWidth    = 50.0f;
            static float OrthoHeight   = 50.0f;
            static float OrthoNear     = 1.0f;
            static float OrthoFar      = 50.0f;

            ImGui::DragFloat("Ortho width", &OrthoWidth);
            ImGui::DragFloat("Ortho height", &OrthoHeight);
            ImGui::DragFloat("Ortho near", &OrthoNear);
            ImGui::DragFloat("Ortho far", &OrthoFar);

            proj = math_OrthographicRH(OrthoWidth, OrthoHeight, OrthoNear, OrthoFar);
            //proj = math_Transpose(proj);

            const gfx_RenderTarget* prev = gfx_RenderTarget_GetActiveRTV();
            m_depthRT.Bind();
            m_depthRT.Clear();
            m_world->Draw(view, proj, game_RenderPass::DEPTH, false);
            if (prev == nullptr) {
                gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
            } else {
                prev->Bind();
            }

            ImTextureID depthTex = m_depthRT.GetNativeTexture();
            ImGui::Image(depthTex, ImVec2(m_depthRT.GetWidth(), m_depthRT.GetHeight()));

        } else if (lmanager->HasPointLight(entity)) {
            game_PointLightId lid     = lmanager->GetPointLightId(entity);
            game_PointLight   light   = lmanager->GetPointLight(lid);
            bool              changed = false;
            changed |= ImGui::DragFloat("Radius", &light.radius);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            if (changed) {
                lmanager->SetPointLight(lid, light);
            }
        }
    }

    // ===============================
    // edit_CommandCreateDirectionalLight
    // ===============================
    edit_CommandCreateDirectionalLight::edit_CommandCreateDirectionalLight(game_World* world)
        : m_world(world)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreateDirectionalLight::Do()
    {
        if (m_createdEntity != game_EntityId_Invalid) {
            core_Assert(!m_world->GetEntityManager()->IsEntityAlive(m_createdEntity));
            m_world->GetEntityManager()->CreateEntity(m_createdEntity);
            m_world->InsertSerializedEntity(m_sentity, m_createdEntity);
        } else {
            m_createdEntity = m_world->GetEntityManager()->CreateEntity();
            std::stringstream ss;
            ss << "DirLight [" << m_createdEntity.id << "]";
            m_world->GetEntityManager()->SetName(m_createdEntity, ss.str().c_str());

            m_world->GetTransformManager()->CreateTransform(m_createdEntity);

            game_DirectionalLight dlight;
            dlight.direction = math_Vec3(0, 0, -1);
            dlight.color     = math_Vec3::One();
            dlight.strength  = 1.0f;
            m_world->GetLightManager()->CreateDirectionalLight(m_createdEntity, dlight);
        }
    }

    void
    edit_CommandCreateDirectionalLight::Undo()
    {
        core_Assert(m_createdEntity != game_EntityId_Invalid);
        m_sentity = m_world->SerializeEntity(m_createdEntity);
        m_world->GetEntityManager()->DestroyEntity(m_createdEntity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateDirectionalLight::Create(game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateDirectionalLight(world));
    }


    // ===============================
    // edit_CommandCreatePointLight
    // ===============================
    edit_CommandCreatePointLight::edit_CommandCreatePointLight(game_World* world)
        : m_world(world)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreatePointLight::Do()
    {
        if (m_createdEntity != game_EntityId_Invalid) {
            core_Assert(!m_world->GetEntityManager()->IsEntityAlive(m_createdEntity));
            m_world->GetEntityManager()->CreateEntity(m_createdEntity);
            m_world->InsertSerializedEntity(m_sentity, m_createdEntity);
        } else {
            m_createdEntity = m_world->GetEntityManager()->CreateEntity();
            std::stringstream ss;
            ss << "PointLight [" << m_createdEntity.id << "]";
            m_world->GetEntityManager()->SetName(m_createdEntity, ss.str().c_str());

            m_world->GetTransformManager()->CreateTransform(m_createdEntity);

            game_PointLight plight;
            plight.radius = 10.0f;
            plight.color  = math_Vec3::One();
            m_world->GetLightManager()->CreatePointLight(m_createdEntity, plight);
        }
    }

    void
    edit_CommandCreatePointLight::Undo()
    {
        core_Assert(m_createdEntity != game_EntityId_Invalid);
        m_sentity = m_world->SerializeEntity(m_createdEntity);
        m_world->GetEntityManager()->DestroyEntity(m_createdEntity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreatePointLight::Create(game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreatePointLight(world));
    }
} // namespace pge