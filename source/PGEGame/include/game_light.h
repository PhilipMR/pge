#ifndef PGE_GAME_GAME_LIGHT_H
#define PGE_GAME_GAME_LIGHT_H

#include "game_entity.h"
#include <math_vec3.h>
#include <unordered_map>

namespace pge
{
    struct game_PointLight {
        math_Vec3 position;
        math_Vec3 color;
        float     radius;
        float     strength;
    };

    class game_LightManager {
        std::unordered_map<game_Entity, game_PointLight> m_pointLightMap;

    public:
        void CreatePointLight(const game_Entity& entity, const game_PointLight& light);
        bool HasPointLight(const game_Entity& entity) const;
        game_PointLight GetPointLight(const game_Entity& entity) const;

        void BindLightCBuffers();
    };
} // namespace pge

#endif