#ifndef PGE_GAME_GAME_LIGHT_H
#define PGE_GAME_GAME_LIGHT_H

#include "game_entity.h"
#include "game_transform.h"
#include <math_vec2.h>
#include <math_vec3.h>
#include <unordered_map>
#include <memory>

namespace pge
{
    struct game_PointLight {
        game_Entity entity;
        math_Vec3   color;
        float       radius;
        float       strength;
    };

    using game_PointLightId                               = unsigned;
    constexpr game_PointLightId game_PointLightId_Invalid = -1;

    class game_TransformManager;
    class game_LightManager {
        std::unordered_map<game_Entity, game_PointLightId> m_pointLightMap;
        std::unique_ptr<game_PointLight[]>                 m_pointLights;
        size_t                                             m_numPointLights;

    public:
        game_LightManager(size_t capacity);
        void              GarbageCollect(const game_EntityManager& entityManager);
        void              CreatePointLight(const game_Entity& entity, const game_PointLight& light);
        void              DestroyPointLight(const game_PointLightId& id);
        bool              HasPointLight(const game_Entity& entity) const;
        game_PointLightId GetPointLightId(const game_Entity& entity) const;
        game_PointLight   GetPointLight(const game_PointLightId& id) const;
        game_Entity       GetEntity(const game_PointLightId& lid) const;

        game_PointLightId HoverSelect(const game_TransformManager& tm,
                                      const math_Vec2&             hoverPosNorm,
                                      const math_Vec2&             rectSize,
                                      const math_Mat4x4&           view,
                                      const math_Mat4x4&           proj) const;

        friend std::ostream& operator<<(std::ostream& os, const game_LightManager& lm);
        friend std::istream& operator>>(std::istream& is, game_LightManager& lm);
    };
} // namespace pge

#endif