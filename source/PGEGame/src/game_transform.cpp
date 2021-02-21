#include "../include/game_transform.h"
#include <diag_assert.h>

namespace pge
{
    game_TransformManager::game_TransformManager(size_t capacity)
        : m_size(0)
        , m_capacity(capacity)
    {
        diag_Assert(m_capacity > 0);
        size_t elementSize = sizeof(game_Entity) + 2 * sizeof(math_Mat4x4) + 4 * sizeof(game_TransformId);
        size_t bufferSize  = elementSize * m_capacity;

        m_buffer     = malloc(bufferSize);
        m_entity     = reinterpret_cast<game_Entity*>(m_buffer);
        m_local      = reinterpret_cast<math_Mat4x4*>(m_entity + capacity);
        m_world      = m_local + capacity;
        m_parent     = reinterpret_cast<game_TransformId*>(m_world + capacity);
        m_firstChild = m_parent + capacity;
        m_next       = m_firstChild + capacity;
        m_prev       = m_next + capacity;
    }

    game_TransformManager::~game_TransformManager()
    {
        free(m_buffer);
    }

    game_TransformId
    game_TransformManager::CreateTransform(const game_Entity& entity, const math_Mat4x4& xform)
    {
        diag_Assert(!HasTransform(entity));
        diag_Assert(m_size < m_capacity);
        game_TransformId tid = m_size++;

        m_entity[tid]     = entity.id;
        m_local[tid]      = xform;
        m_world[tid]      = xform;
        m_parent[tid]     = game_TransformId_Invalid;
        m_firstChild[tid] = game_TransformId_Invalid;
        m_next[tid]       = game_TransformId_Invalid;
        m_prev[tid]       = game_TransformId_Invalid;

        m_entityMap.insert(std::make_pair(entity, tid));
        return tid;
    }

    void
    game_TransformManager::CreateTransforms(const game_Entity* entities, size_t numEntities, game_TransformId* destBuf)
    {
        for (size_t i = 0; i < numEntities; ++i) {
            destBuf[i] = CreateTransform(entities[i], math_Mat4x4());
        }
    }

    void
    game_TransformManager::DestroyTransform(const game_TransformId& id)
    {
        diag_Assert(id < m_size);

        const game_TransformId& delId  = id;
        const game_Entity&      delEnt = m_entity[delId];

        const game_TransformId& lastId  = m_size - 1;
        const game_Entity&      lastEnt = m_entity[lastId];

        if (delId != lastId) {
            game_TransformId delChildId = m_firstChild[delId];
            while (delChildId != game_TransformId_Invalid) {
                m_parent[delChildId] = delId;
                delChildId           = m_next[delChildId];
            }

            m_entity[delId]     = m_entity[lastId];
            m_local[delId]      = m_local[lastId];
            m_world[delId]      = m_world[lastId];
            m_parent[delId]     = m_parent[lastId];
            m_firstChild[delId] = m_firstChild[lastId];
            m_next[delId]       = m_next[lastId];
            m_prev[delId]       = m_prev[lastId];
            m_entityMap.erase(delEnt);
            m_entityMap[lastEnt] = delId;
        }
        m_size--;
    }

    void
    game_TransformManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (size_t aliveStreak = 0; m_size > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_size;
            if (!entityManager.IsEntityAlive(m_entity[randIdx])) {
                DestroyTransform(randIdx);
                aliveStreak = 0;
            } else {
                aliveStreak++;
            }
        }
    }

    bool
    game_TransformManager::HasTransform(const game_Entity& entity) const
    {
        return m_entityMap.find(entity) != m_entityMap.end();
    }

    game_TransformId
    game_TransformManager::GetTransformId(const game_Entity& entity) const
    {
        diag_Assert(HasTransform(entity));
        return m_entityMap.at(entity);
    }
    //
    //    void
    //    game_TransformManager::Translate(const game_TransformId& id, const math_Vec3& translation)
    //    {}
    //
    void
    game_TransformManager::Rotate(const game_TransformId& id, const math_Vec3& axis, float degrees)
    {
        diag_Assert(id < m_size);
        math_Mat4x4 newLocal = m_local[id] * math_CreateRotationMatrix(math_QuaternionFromAxisAngle(axis, degrees));
        SetLocal(id, newLocal);
    }
    //
    //    void
    //    game_TransformManager::Scale(const game_TransformId& id, const math_Vec3& scale)
    //    {}

    //    void
    //    game_TransformManager::SetLocalPosition(const game_TransformId& id, const math_Vec3& position)
    //    {}
    //
    //    void
    //    game_TransformManager::SetLocalRotation(const game_TransformId& id, const math_Quat& rotation)
    //    {}
    //
    //    void
    //    game_TransformManager::SetLocalScale(const game_TransformId& id, const math_Vec3& scale)
    //    {}

    void
    game_TransformManager::SetLocal(const game_TransformId& id, const math_Mat4x4& matrix)
    {
        diag_Assert(id < m_size);
        m_local[id]                  = matrix;
        game_TransformId parentId    = m_parent[id];
        math_Mat4x4      parentWorld = parentId == game_TransformId_Invalid ? math_Mat4x4() : m_world[parentId];
        Transform(id, parentWorld);
    }

    //    math_Vec3
    //    game_TransformManager::GetLocalPosition(const game_TransformId& id) const
    //    {}
    //
    //    math_Quat
    //    game_TransformManager::GetLocalRotation(const game_TransformId& id) const
    //    {}
    //
    //    math_Vec3
    //    game_TransformManager::GetLocalScale(const game_TransformId& id) const
    //    {}

    math_Mat4x4
    game_TransformManager::GetLocal(const game_TransformId& id) const
    {
        diag_Assert(id < m_size);
        return m_local[id];
    }

    //    math_Vec3
    //    game_TransformManager::GetWorldPosition(const game_TransformId& id) const
    //    {}
    //
    //    math_Quat
    //    game_TransformManager::GetWorldRotation(const game_TransformId& id) const
    //    {}
    //
    //    math_Vec3
    //    game_TransformManager::GetWorldScale(const game_TransformId& id) const
    //    {}

    math_Mat4x4
    game_TransformManager::GetWorld(const game_TransformId& id) const
    {
        diag_Assert(id < m_size);
        return m_world[id];
    }

    void
    game_TransformManager::Transform(const game_TransformId& id, const math_Mat4x4& parent)
    {
        m_world[id]            = parent * m_local[id];
        game_TransformId child = m_firstChild[id];
        while (child != game_TransformId_Invalid) {
            Transform(child, m_world[id]);
            child = m_next[child];
        }
    }
} // namespace pge