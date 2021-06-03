#ifndef PGE_GAME_GAME_TRANSFORM_H
#define PGE_GAME_GAME_TRANSFORM_H

#include "game_entity.h"
#include <math_mat4x4.h>
#include <unordered_map>

namespace pge
{
    using game_TransformId                         = unsigned;
    static const unsigned game_TransformId_Invalid = -1;

    struct LocalTransformData;
    class game_TransformManager {
        std::unordered_map<game_Entity, game_TransformId> m_entityMap;

        size_t m_capacity;
        void*  m_buffer;

        game_Entity*        m_entity;
        LocalTransformData* m_localData;
        math_Mat4x4*        m_local;
        math_Mat4x4*        m_world;
        game_TransformId*   m_parent;
        game_TransformId*   m_firstChild;
        game_TransformId*   m_next;
        game_TransformId*   m_prev;

        void                   AllocateBuffers(size_t capacity);
        game_TransformManager& operator=(const game_TransformManager& rhs) = delete;

    public:
        explicit game_TransformManager(size_t capacity);
        ~game_TransformManager();

        game_TransformId CreateTransform(const game_Entity& entity,
                                         const math_Vec3&   position = math_Vec3::Zero(),
                                         const math_Quat&   rotation = math_Quat(),
                                         const math_Vec3&   scale    = math_Vec3::One());
        void             CreateTransforms(const game_Entity* entities, size_t numEntities, game_TransformId* destBuf);
        void             DestroyTransform(const game_TransformId& id);
        void             GarbageCollect(const game_EntityManager& entityManager);

        bool             HasTransform(const game_Entity& entity) const;
        game_TransformId GetTransformId(const game_Entity& entity) const;

        void Translate(const game_TransformId& id, const math_Vec3& translation);
        void Rotate(const game_TransformId& id, const math_Vec3& axis, float degrees);
        void Scale(const game_TransformId& id, const math_Vec3& scale);

        void SetLocal(const game_TransformId& id, const math_Vec3& position, const math_Quat& rotation, const math_Vec3& scale);
        void SetLocalPosition(const game_TransformId& id, const math_Vec3& position);
        void SetLocalRotation(const game_TransformId& id, const math_Quat& rotation);
        void SetLocalScale(const game_TransformId& id, const math_Vec3& scale);
        void SetLocalForward(const game_TransformId& id, const math_Vec3& forward, const math_Vec3& up);
        void SetLocalLookAt(const game_TransformId& id, const math_Vec3& position, const math_Vec3& target, const math_Vec3& up);


        math_Mat4x4 GetLocal(const game_TransformId& id) const;
        math_Vec3   GetLocalPosition(const game_TransformId& id) const;
        math_Quat   GetLocalRotation(const game_TransformId& id) const;
        math_Vec3   GetLocalScale(const game_TransformId& id) const;
        math_Vec3   GetLocalRight(const game_TransformId& id) const;
        math_Vec3   GetLocalUp(const game_TransformId& id) const;
        math_Vec3   GetLocalForward(const game_TransformId& id) const;

        math_Vec3   GetWorldPosition(const game_TransformId& id) const;
        math_Quat   GetWorldRotation(const game_TransformId& id) const;
        math_Vec3   GetWorldScale(const game_TransformId& id) const;
        math_Mat4x4 GetWorld(const game_TransformId& id) const;

        void SerializeEntity(std::ostream& os, const game_Entity& entity) const;
        void InsertSerializedEntity(std::istream& is, const game_Entity& entity);

        friend std::ostream& operator<<(std::ostream& os, const game_TransformManager& tm);
        friend std::istream& operator>>(std::istream& is, game_TransformManager& tm);

    private:
        void Transform(const game_TransformId& id, const math_Mat4x4& parent);
        void SetLocal(const game_TransformId& id, const math_Mat4x4& matrix);
    };
} // namespace pge

#endif