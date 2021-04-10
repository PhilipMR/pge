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
        game_PointLightId pid = m_lightManager->GetPointLightId(entity);
        diag_Assert(pid != game_PointLightId_Invalid);
        game_PointLight light = m_lightManager->GetPointLight(pid);

        bool changed = 0;
        changed |= ImGui::DragFloat("Radius", &light.radius);
        changed |= ImGui::ColorPicker3("Color", &light.color[0]);
        if (changed) {
            m_lightManager->SetPointLight(pid, light);
        }
    }
} // namespace pge