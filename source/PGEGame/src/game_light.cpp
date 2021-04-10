#include "../include/game_light.h"
#include <math_rect.h>
#include <diag_assert.h>
#include <iostream>

namespace pge
{
    game_LightManager::game_LightManager(size_t capacity)
        : m_pointLights(new game_PointLight[capacity])
        , m_numPointLights(0)
    {}

    void
    game_LightManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (size_t aliveStreak = 0; m_numPointLights > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_numPointLights;
            if (!entityManager.IsEntityAlive(m_pointLights[randIdx].entity)) {
                DestroyPointLight(randIdx);
                aliveStreak = 0;
            } else {
                aliveStreak++;
            }
        }
    }

    void
    game_LightManager::CreatePointLight(const game_Entity& entity, const game_PointLight& light)
    {
        diag_Assert(!HasPointLight(entity));
        game_PointLightId lid = m_numPointLights++;
        m_pointLightMap.insert(std::make_pair(entity, lid));
        m_pointLights[lid] = light;
        m_pointLights[lid].entity = entity;
    }

    void
    game_LightManager::DestroyPointLight(const game_PointLightId& id)
    {
        diag_Assert(id < m_numPointLights);
        game_Entity       entity = m_pointLights[id].entity;
        game_PointLightId lastId = m_numPointLights - 1;
        m_pointLightMap.erase(m_pointLightMap.find(entity));
        if (id != lastId) {
            m_pointLights[id]           = m_pointLights[lastId];
            game_Entity lastEntity      = m_pointLights[lastId].entity;
            m_pointLightMap[lastEntity] = id;
        }
        m_numPointLights--;
    }

    bool
    game_LightManager::HasPointLight(const game_Entity& entity) const
    {
        return m_pointLightMap.find(entity) != m_pointLightMap.end();
    }

    game_PointLightId
    game_LightManager::GetPointLightId(const game_Entity& entity) const
    {
        return m_pointLightMap.find(entity)->second;
    }

    game_PointLight
    game_LightManager::GetPointLight(const game_PointLightId& id) const
    {
        diag_Assert(id < m_numPointLights);
        return m_pointLights[id];
    }

    const game_PointLight*
    game_LightManager::GetPointLights(size_t* count) const
    {
        *count = m_numPointLights;
        return &m_pointLights[0];
    }

    void
    game_LightManager::SetPointLight(const game_PointLightId& id, const game_PointLight& light)
    {
        diag_Assert(id < m_numPointLights);
        m_pointLights[id] = light;
    }

    game_PointLightId
    game_LightManager::HoverSelect(const game_TransformManager& tm, const math_Vec2& hoverPosNorm, const math_Vec2& rectSize, const math_Mat4x4& view, const math_Mat4x4& proj) const
    {
        for (size_t i = 0; i < m_numPointLights; ++i) {
            const game_PointLight& plight    = m_pointLights[i];
            math_Vec3              worldPos;
            if (tm.HasTransform(plight.entity)) {
                auto tid = tm.GetTransformId(plight.entity);
                worldPos = tm.GetWorldPosition(tid);
            }
            math_Vec4              viewPos   = view * math_Vec4(worldPos, 1);
            math_Vec4              screenPos = proj * viewPos;
            math_Vec2              screenPosXY(screenPos.x / screenPos.w, -screenPos.y / screenPos.w);
            // Map from [-1,1] to [0,1] to match hoverPosNorm
            screenPosXY += math_Vec2::One();
            screenPosXY /= 2;

            const math_Vec2 hsize      = rectSize / 2;
            math_Vec4       topLeft    = proj * (viewPos + math_Vec4(-hsize.x, -hsize.y, 0, 0));
            math_Vec4       topLeftHom = topLeft / topLeft.w;
            topLeftHom += math_Vec4::One();
            topLeftHom /= 2;

            math_Vec4 botRight    = proj * (viewPos + math_Vec4(hsize.x, hsize.y, 0, 0));
            math_Vec4 botRightHom = botRight / botRight.w;
            botRightHom += math_Vec4::One();
            botRightHom /= 2;

            math_Vec2 screenRectSize(botRightHom.x - topLeftHom.x, botRightHom.y - topLeftHom.y);
            math_Vec2 hscreenRectSize = screenRectSize / 2;
            math_Vec2 screenRectPos   = screenPosXY - hscreenRectSize;
            math_Rect billboard(screenRectPos, screenRectSize);
            if (billboard.Intersects(math_Vec2(hoverPosNorm.x, hoverPosNorm.y))) {
                return i;
            }
        }
        return game_PointLightId_Invalid;
    }


    constexpr unsigned SERIALIZE_VERSION = 1;

    std::ostream&
    operator<<(std::ostream& os, const game_LightManager& lm)
    {
        unsigned version = SERIALIZE_VERSION;
        os.write((const char*)&version, sizeof(version));
        os.write((const char*)&lm.m_numPointLights, sizeof(lm.m_numPointLights));
        os.write((const char*)&lm.m_pointLights[0], lm.m_numPointLights * sizeof(game_PointLight));
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_LightManager& lm)
    {
        unsigned version = 0;
        is.read((char*)&version, sizeof(version));
        is.read((char*)&lm.m_numPointLights, sizeof(lm.m_numPointLights));
        is.read((char*)&lm.m_pointLights[0], lm.m_numPointLights * sizeof(game_PointLight));
        lm.m_pointLightMap.clear();
        for (game_TransformId i = 0; i < lm.m_numPointLights; ++i) {
            lm.m_pointLightMap.insert(std::make_pair(lm.m_pointLights[i].entity, i));
        }
        return is;
    }

} // namespace pge