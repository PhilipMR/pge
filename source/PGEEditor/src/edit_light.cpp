#include "../include/edit_light.h"
#include <imgui/imgui.h>
#include <sstream>

namespace pge
{
    edit_LightEditor::edit_LightEditor(game_LightManager* lm)
        : m_lightManager(lm)
    {}

    void
    edit_LightEditor::UpdateAndDraw(const game_Entity& entity)
    {
        if (!ImGui::CollapsingHeader("Light"))
            return;

        core_Assert(m_lightManager->HasDirectionalLight(entity) || m_lightManager->HasPointLight(entity));
        if (m_lightManager->HasDirectionalLight(entity)) {
            game_DirectionalLightId lid     = m_lightManager->GetDirectionalLightId(entity);
            game_DirectionalLight   light   = m_lightManager->GetDirectionalLight(lid);
            bool                    changed = false;
            changed |= ImGui::DragFloat3("Direction", &light.direction[0]);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            changed |= ImGui::DragFloat("Strength", &light.strength);
            if (changed) {
                m_lightManager->SetDirectionalLight(lid, light);
            }
        } else if (m_lightManager->HasPointLight(entity)) {
            game_PointLightId lid     = m_lightManager->GetPointLightId(entity);
            game_PointLight   light   = m_lightManager->GetPointLight(lid);
            bool              changed = false;
            changed |= ImGui::DragFloat("Radius", &light.radius);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            if (changed) {
                m_lightManager->SetPointLight(lid, light);
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