#include "../include/game_transform.h"
#include <core_assert.h>
#include <iostream>

namespace pge
{
    struct LocalTransformData {
        math_Vec3 position;
        math_Quat rotation;
        math_Vec3 scale;
    };
    static const size_t TRANSFORM_ELEMENT_SIZE
        = sizeof(game_Entity) + sizeof(LocalTransformData) + 2 * sizeof(math_Mat4x4) + 4 * sizeof(game_TransformId);

    void
    game_TransformManager::AllocateBuffers(size_t capacity)
    {
        if (m_buffer != nullptr)
            free(m_buffer);
        core_Assert(capacity > 0);
        m_capacity        = capacity;
        size_t bufferSize = TRANSFORM_ELEMENT_SIZE * m_capacity;

        m_buffer     = malloc(bufferSize);
        m_entity     = reinterpret_cast<game_Entity*>(m_buffer);
        m_localData  = reinterpret_cast<LocalTransformData*>(m_entity + capacity);
        m_local      = reinterpret_cast<math_Mat4x4*>(m_localData + capacity);
        m_world      = m_local + capacity;
        m_parent     = reinterpret_cast<game_TransformId*>(m_world + capacity);
        m_firstChild = m_parent + capacity;
        m_next       = m_firstChild + capacity;
        m_prev       = m_next + capacity;
    }

    game_TransformManager::game_TransformManager(size_t capacity)
        : m_capacity(capacity)
        , m_buffer(nullptr)
    {
        AllocateBuffers(capacity);
    }

    game_TransformManager::~game_TransformManager()
    {
        free(m_buffer);
    }

    game_TransformId
    game_TransformManager::CreateTransform(const game_Entity& entity, const math_Vec3& position, const math_Quat& rotation, const math_Vec3& scale)
    {
        core_Assert(!HasTransform(entity));
        core_Assert(m_entityMap.size() < m_capacity);
        game_TransformId tid = m_entityMap.size();

        m_entity[tid]             = entity.id;
        m_localData[tid].position = position;
        m_localData[tid].rotation = rotation;
        m_localData[tid].scale    = math_Vec3::One();

        math_Mat4x4 xform = math_CreateTransformMatrix(position, rotation, scale);
        m_world[tid]      = xform;
        m_parent[tid]     = game_TransformId_Invalid;
        m_firstChild[tid] = game_TransformId_Invalid;
        m_next[tid]       = game_TransformId_Invalid;
        m_prev[tid]       = game_TransformId_Invalid;

        m_entityMap.insert(std::make_pair(entity, tid));
        SetLocal(tid, xform);

        return tid;
    }

    void
    game_TransformManager::CreateTransforms(const game_Entity* entities, size_t numEntities, game_TransformId* destBuf)
    {
        for (size_t i = 0; i < numEntities; ++i) {
            destBuf[i] = CreateTransform(entities[i], math_Vec3::Zero(), math_Quat(), math_Vec3::One());
        }
    }

    void
    game_TransformManager::DestroyTransform(const game_TransformId& id)
    {
        core_Assert(id < m_entityMap.size());

        const game_TransformId& delId  = id;
        const game_Entity       delEnt = m_entity[delId];

        // Remove from current parent (if any)
        SetParent(delId, game_TransformId_Invalid);

        // Detach from children
        auto child = m_firstChild[delId];
        while (child != game_TransformId_Invalid) {
            m_parent[child] = game_TransformId_Invalid;
            m_prev[child]   = game_TransformId_Invalid;
            auto next       = m_next[child];
            m_next[child]   = game_TransformId_Invalid;
            child           = next;
        }
        m_firstChild[delId] = game_TransformId_Invalid;

        // Swap last with current data in buffers and remap
        const game_TransformId& lastId  = m_entityMap.size() - 1;
        const game_Entity       lastEnt = m_entity[lastId];
        if (delId != lastId) {
            m_entity[delId]    = m_entity[lastId];
            m_localData[delId] = m_localData[lastId];
            m_local[delId]     = m_local[lastId];
            m_world[delId]     = m_world[lastId];

            // Update the lastId-childrens parent
            game_TransformId c = m_firstChild[lastId];
            while (c != game_TransformId_Invalid) {
                auto next = m_next[c];
                SetParent(c, delId);
                c = next;
            }

            // Update lastId's parent
            game_TransformId p = m_parent[lastId];
            SetParent(lastId, game_TransformId_Invalid); // Remove lastId from parent
            SetParent(delId, p);

            m_entityMap[lastEnt] = delId;
        }
        m_entityMap.erase(delEnt);
    }

    void
    game_TransformManager::GarbageCollect(const game_EntityManager& emanager)
    {
        for (size_t aliveStreak = 0; m_entityMap.size() > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_entityMap.size();
            if (!emanager.IsEntityAlive(m_entity[randIdx])) {
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
        core_Assert(HasTransform(entity));
        return m_entityMap.at(entity);
    }

    void
    game_TransformManager::Translate(const game_TransformId& id, const math_Vec3& translation)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].position += translation;
        for (size_t i = 0; i < 3; ++i)
            m_local[id][i][3] = m_localData[id].position[i];
        SetLocal(id, m_local[id]);
    }

    void
    game_TransformManager::Rotate(const game_TransformId& id, const math_Vec3& axis, float degrees)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].rotation *= math_QuatFromAxisAngle(axis, degrees);
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::Scale(const game_TransformId& id, const math_Vec3& scale)
    {
        core_Assert(id < m_entityMap.size());
        for (size_t i = 0; i < 3; ++i)
            m_localData[id].scale[i] *= scale[i];
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::SetParent(const game_TransformId& child, const game_TransformId& parent)
    {
        core_Assert(child != parent);
        core_Assert(child < m_entityMap.size());
        if (m_parent[child] == parent)
            return;

        // Adjust local matrix s.t. the world transform remains unchanged.
        {
            // World[oldparentofi] * Local[i] = World[newparentofi] * X * Local[i]
            // => World[oldparentofi] = World[newparentofi] * X
            // => X = inverse(World[newparentofi])) * World[oldparentofi]
            math_Mat4x4 oldpar;
            if (m_parent[child] != game_TransformId_Invalid) {
                oldpar = m_world[m_parent[child]];
            }

            math_Mat4x4 newparinv;
            if (parent != game_TransformId_Invalid) {
                core_Verify(math_Invert(m_world[parent], &newparinv));
            }

            math_Mat4x4 X  = newparinv * oldpar;
            m_local[child] = X * m_local[child];
            math_DecomposeMatrix(m_local[child], &m_localData[child].position, &m_localData[child].rotation, &m_localData[child].scale);
        }


        // Remove from current parent
        if (m_parent[child] != game_TransformId_Invalid) {
            game_TransformId curParent = m_parent[child];
            if (child == m_firstChild[curParent]) {
                m_firstChild[curParent] = m_next[child];
                if (m_next[child] != game_TransformId_Invalid) {
                    m_prev[m_next[child]] = game_TransformId_Invalid;
                }
            } else {
                game_TransformId t = m_firstChild[curParent];
                while (t != child)
                    t = m_next[t];
                if (m_prev[t] != game_TransformId_Invalid)
                    m_next[m_prev[t]] = m_next[t];
                if (m_next[t] != game_TransformId_Invalid)
                    m_prev[m_next[t]] = m_prev[t];
            }
        }

        // Assign to new parent
        if (parent != game_TransformId_Invalid) {
            if (m_firstChild[parent] == game_EntityId_Invalid) {
                m_firstChild[parent] = child;
                m_prev[child]        = game_TransformId_Invalid;
                m_next[child]        = game_TransformId_Invalid;
            } else {
                const game_TransformId prevFirst = m_firstChild[parent];
                m_firstChild[parent]             = child;
                m_next[child]                    = prevFirst;
                m_prev[child]                    = game_TransformId_Invalid;
                m_prev[prevFirst]                = child;
            }
        }

        m_parent[child] = parent;
    }

    void
    game_TransformManager::SetLocal(const game_TransformId& id, const math_Vec3& position, const math_Quat& rotation, const math_Vec3& scale)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].position = position;
        m_localData[id].rotation = rotation;
        m_localData[id].scale    = scale;
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::SetLocalPosition(const game_TransformId& id, const math_Vec3& position)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].position = position;
        for (size_t i = 0; i < 3; ++i)
            m_local[id][i][3] = m_localData[id].position[i];
        SetLocal(id, m_local[id]);
    }

    void
    game_TransformManager::SetLocalRotation(const game_TransformId& id, const math_Quat& rotation)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].rotation = rotation;
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::SetLocalScale(const game_TransformId& id, const math_Vec3& scale)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].scale = scale;
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::SetLocalForward(const game_TransformId& id, const math_Vec3& forward, const math_Vec3& up)
    {
        core_Assert(id < m_entityMap.size());

        const math_Vec3 originalFwd(0, 1, 0);
        const math_Vec3 cross = math_Cross(originalFwd, forward);
        const float     dot   = math_Dot(originalFwd, forward);

        // Compare the new forward vector with the original, there are 3 cases:
        if (dot >= 1) {
            // 1. Same direction, keep rotation as default.
            m_localData[id].rotation = math_Quat();
        } else if (dot <= -1) {
            // 2. Opposite direction, rotate 180 degrees.
            m_localData[id].rotation = math_QuatFromAxisAngle(up, 180.0f);
        } else {
            // 3. Not parallel, determine corresponding rotation.
            float qw                 = sqrtf(math_LengthSquared(originalFwd) * math_LengthSquared(forward)) + dot;
            m_localData[id].rotation = math_Normalize(math_Quat(qw, cross));
        }
        SetLocal(id, math_CreateTransformMatrix(m_localData[id].position, m_localData[id].rotation, m_localData[id].scale));
    }

    void
    game_TransformManager::SetLocalLookAt(const game_TransformId& id, const math_Vec3& position, const math_Vec3& target, const math_Vec3& up)
    {
        core_Assert(id < m_entityMap.size());
        m_localData[id].position = position;
        m_localData[id].scale    = math_Vec3::One();
        SetLocalForward(id, math_Normalize(target - position), up);
    }

    const game_Entity&
    game_TransformManager::GetEntity(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_entity[id];
    }

    const game_TransformId&
    game_TransformManager::GetParent(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_parent[id];
    }

    bool
    game_TransformManager::IsAncestor(const game_TransformId& id, const game_TransformId& ancestor) const
    {
        core_Assert(id < m_entityMap.size());
        core_Assert(ancestor < m_entityMap.size());
        game_TransformId parent = m_parent[id];
        while (parent != game_TransformId_Invalid) {
            if (parent == ancestor)
                return true;
            parent = m_parent[parent];
        }
        return false;
    }

    const game_TransformId&
    game_TransformManager::GetFirstChild(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_firstChild[id];
    }

    const game_TransformId&
    game_TransformManager::GetNextSibling(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_next[id];
    }

    const game_TransformId&
    game_TransformManager::GetPreviousSibling(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_prev[id];
    }

    math_Mat4x4
    game_TransformManager::GetLocalMatrix(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_local[id];
    }

    math_Vec3
    game_TransformManager::GetLocalPosition(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_localData[id].position;
    }

    math_Quat
    game_TransformManager::GetLocalRotation(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_localData[id].rotation;
    }

    math_Vec3
    game_TransformManager::GetLocalScale(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_localData[id].scale;
    }

    math_Mat4x4
    game_TransformManager::GetWorldMatrix(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        return m_world[id];
    }

    math_Vec3
    game_TransformManager::GetWorldPosition(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        const math_Mat4x4& world = m_world[id];
        return math_Vec3(world[0][3], world[1][3], world[2][3]);
    }

    math_Quat
    game_TransformManager::GetWorldRotation(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        game_TransformId pid = m_parent[id];
        math_Quat        parRot;
        if (pid != game_TransformId_Invalid) {
            parRot = GetWorldRotation(pid);
        }
        return parRot * m_localData[id].rotation;
    }

    math_Vec3
    game_TransformManager::GetWorldScale(const game_TransformId& id) const
    {
        core_Assert(id < m_entityMap.size());
        game_TransformId pid = m_parent[id];
        math_Vec3        parScale;
        if (pid != game_TransformId_Invalid) {
            parScale = GetWorldScale(pid);
        }
        math_Vec3 scale = m_localData[id].scale;
        for (size_t i = 0; i < 3; ++i)
            scale[i] *= parScale[i];
        return scale;
    }


    constexpr unsigned SERIALIZE_VERSION = 1;

    void
    game_TransformManager::SerializeEntity(std::ostream& os, const game_Entity& entity) const
    {
        game_TransformId          tid       = m_entityMap.at(entity);
        const LocalTransformData& localData = m_localData[tid];
        os.write((const char*)&localData.position, sizeof(localData.position));
        os.write((const char*)&localData.rotation, sizeof(localData.rotation));
        os.write((const char*)&localData.scale, sizeof(localData.scale));
    }

    void
    game_TransformManager::InsertSerializedEntity(std::istream& is, const game_Entity& entity)
    {
        if (!HasTransform(entity)) {
            CreateTransform(entity);
        }
        game_TransformId    tid       = m_entityMap[entity];
        LocalTransformData& localData = m_localData[tid];
        is.read((char*)&localData.position, sizeof(localData.position));
        is.read((char*)&localData.rotation, sizeof(localData.rotation));
        is.read((char*)&localData.scale, sizeof(localData.scale));
        SetLocal(tid, math_CreateTransformMatrix(localData.position, localData.rotation, localData.scale));
    }

    std::ostream&
    operator<<(std::ostream& os, const game_TransformManager& tm)
    {
        unsigned numComponents = tm.m_entityMap.size();

        unsigned version = SERIALIZE_VERSION;
        os.write((const char*)&version, sizeof(version));
        os.write((const char*)&numComponents, sizeof(numComponents));

        os.write((const char*)tm.m_entity, sizeof(tm.m_entity[0]) * numComponents);
        os.write((const char*)tm.m_localData, sizeof(tm.m_localData[0]) * numComponents);
        os.write((const char*)tm.m_world, sizeof(tm.m_world[0]) * numComponents);
        os.write((const char*)tm.m_parent, sizeof(tm.m_parent[0]) * numComponents);
        os.write((const char*)tm.m_firstChild, sizeof(tm.m_firstChild[0]) * numComponents);
        os.write((const char*)tm.m_next, sizeof(tm.m_next[0]) * numComponents);
        os.write((const char*)tm.m_prev, sizeof(tm.m_prev[0]) * numComponents);

        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_TransformManager& tm)
    {
        unsigned version = 0;
        is.read((char*)&version, sizeof(version));

        unsigned numTransforms = 0;
        is.read((char*)&numTransforms, sizeof(numTransforms));

        if (tm.m_capacity < numTransforms) {
            tm.AllocateBuffers(numTransforms);
        }

        is.read((char*)tm.m_entity, sizeof(tm.m_entity[0]) * numTransforms);
        is.read((char*)tm.m_localData, sizeof(tm.m_localData[0]) * numTransforms);
        is.read((char*)tm.m_world, sizeof(tm.m_world[0]) * numTransforms);
        is.read((char*)tm.m_parent, sizeof(tm.m_parent[0]) * numTransforms);
        is.read((char*)tm.m_firstChild, sizeof(tm.m_firstChild[0]) * numTransforms);
        is.read((char*)tm.m_next, sizeof(tm.m_next[0]) * numTransforms);
        is.read((char*)tm.m_prev, sizeof(tm.m_prev[0]) * numTransforms);

        tm.m_entityMap.clear();
        for (unsigned i = 0; i < numTransforms; ++i) {
            tm.m_entityMap.insert(std::make_pair<>(tm.m_entity[i], i));
        }

        for (size_t i = 0; i < numTransforms; ++i) {
            LocalTransformData data = tm.m_localData[i];
            tm.SetLocal(i, math_CreateTransformMatrix(data.position, data.rotation, data.scale));
        }

        return is;
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

    void
    game_TransformManager::SetLocal(const game_TransformId& id, const math_Mat4x4& matrix)
    {
        core_Assert(id < m_entityMap.size());
        m_local[id]                  = matrix;
        game_TransformId parentId    = m_parent[id];
        math_Mat4x4      parentWorld = parentId == game_TransformId_Invalid ? math_Mat4x4() : m_world[parentId];
        Transform(id, parentWorld);
    }
} // namespace pge