#include "../include/edit_light.h"
#include <imgui/imgui.h>

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

        diag_Assert(m_lightManager->HasDirectionalLight(entity) || m_lightManager->HasPointLight(entity));
        if (m_lightManager->HasDirectionalLight(entity)) {
            game_DirectionalLightId lid     = m_lightManager->GetDirectionalLightId(entity);
            game_DirectionalLight   light   = m_lightManager->GetDirectionalLight(lid);
            bool                    changed = 0;
            changed |= ImGui::DragFloat3("Direction", &light.direction[0]);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            changed |= ImGui::DragFloat("Strength", &light.strength);
            if (changed) {
                m_lightManager->SetDirectionalLight(lid, light);
            }
        } else if (m_lightManager->HasPointLight(entity)) {
            game_PointLightId lid     = m_lightManager->GetPointLightId(entity);
            game_PointLight   light   = m_lightManager->GetPointLight(lid);
            bool              changed = 0;
            changed |= ImGui::DragFloat("Radius", &light.radius);
            changed |= ImGui::ColorPicker3("Color", &light.color[0]);
            if (changed) {
                m_lightManager->SetPointLight(lid, light);
            }
        }
    }
} // namespace pge