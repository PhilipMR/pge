#include "../include/game_light.h"
#include <diag_assert.h>

namespace pge
{
    void
    game_LightManager::CreatePointLight(const game_Entity& entity, const game_PointLight& light)
    {
        diag_Assert(!HasPointLight(entity));
        m_pointLightMap.insert(std::make_pair(entity, light));
    }

    bool
    game_LightManager::HasPointLight(const game_Entity& entity) const
    {
        return m_pointLightMap.find(entity) != m_pointLightMap.end();
    }

    game_PointLight
    game_LightManager::GetPointLight(const game_Entity& entity) const
    {
        diag_Assert(HasPointLight(entity));
        return m_pointLightMap[entity];
    }

    void
    game_LightManager::BindLightCBuffers()
    {

    }
} // namespace pge