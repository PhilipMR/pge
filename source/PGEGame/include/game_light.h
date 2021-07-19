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
    struct game_DirectionalLight {
        game_Entity entity;
        math_Vec3   color     = math_Vec3::One();
        float       strength  = 1.0f;
        math_Vec3   direction = math_Vec3(0, 0, -1);
    };

    struct game_PointLight {
        game_Entity entity;
        math_Vec3   color;
        float       radius;
    };

    using game_DirectionalLightId                                     = unsigned;
    constexpr game_DirectionalLightId game_DirectionalLightId_Invalid = -1;

    using game_PointLightId                               = unsigned;
    constexpr game_PointLightId game_PointLightId_Invalid = -1;

    class game_TransformManager;
    class game_LightManager {
        game_TransformManager* m_transformManager;

        std::unordered_map<game_Entity, game_DirectionalLightId> m_dirLightMap;
        std::unique_ptr<game_DirectionalLight[]>                 m_dirLights;
        size_t                                                   m_numDirLights;

        std::unordered_map<game_Entity, game_PointLightId> m_pointLightMap;
        std::unique_ptr<game_PointLight[]>                 m_pointLights;
        size_t                                             m_numPointLights;

    public:
        explicit game_LightManager(game_TransformManager* tmanager, size_t capacity);
        void GarbageCollect(const game_EntityManager& entityManager);

        void                         CreateDirectionalLight(const game_Entity& entity, const game_DirectionalLight& light);
        void                         DestroyDirectionalLight(const game_DirectionalLightId& id);
        bool                         HasDirectionalLight(const game_Entity& entity) const;
        game_DirectionalLightId      GetDirectionalLightId(const game_Entity& entity) const;
        game_DirectionalLight        GetDirectionalLight(const game_DirectionalLightId& id) const;
        const game_DirectionalLight* GetDirectionalLights(size_t* count) const;
        void                         SetDirectionalLight(const game_DirectionalLightId& id, const game_DirectionalLight& light);

        void                   CreatePointLight(const game_Entity& entity, const game_PointLight& light);
        void                   DestroyPointLight(const game_PointLightId& id);
        bool                   HasPointLight(const game_Entity& entity) const;
        game_PointLightId      GetPointLightId(const game_Entity& entity) const;
        game_PointLight        GetPointLight(const game_PointLightId& id) const;
        const game_PointLight* GetPointLights(size_t* count) const;
        void                   SetPointLight(const game_PointLightId& id, const game_PointLight& light);

        bool HasLight(const game_Entity& entity) const;

        game_Entity FindLightAtCursor(const math_Vec2&   cursorNorm,
                                      const math_Vec2&   rectSize,
                                      const math_Mat4x4& view,
                                      const math_Mat4x4& proj,
                                      float*             distanceOut) const;

        void SerializeEntity(std::ostream& os, const game_Entity& entity) const;
        void InsertSerializedEntity(std::istream& is, const game_Entity& entity);

        friend std::ostream& operator<<(std::ostream& os, const game_LightManager& lm);
        friend std::istream& operator>>(std::istream& is, game_LightManager& lm);
    };
} // namespace pge

#endif