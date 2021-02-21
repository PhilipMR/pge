#ifndef PGE_GAME_GAME_TRANSFORM_H
#define PGE_GAME_GAME_TRANSFORM_H

#include "game_entity.h"
#include <math_mat4x4.h>
#include <unordered_map>

namespace pge
{
    using game_TransformId                         = unsigned;
    static const unsigned game_TransformId_Invalid = -1;
    class game_TransformManager {
        std::unordered_map<game_Entity, game_TransformId> m_entityMap;

        size_t m_size;
        size_t m_capacity;
        void*  m_buffer;

        game_Entity*      m_entity;
        math_Mat4x4*      m_local;
        math_Mat4x4*      m_world;
        game_TransformId* m_parent;
        game_TransformId* m_firstChild;
        game_TransformId* m_next;
        game_TransformId* m_prev;

    public:
        game_TransformManager(size_t capacity);
        ~game_TransformManager();

        game_TransformId CreateTransform(const game_Entity& entity, const math_Mat4x4& xform = math_Mat4x4::Identity());
        void             CreateTransforms(const game_Entity* entities, size_t numEntities, game_TransformId* destBuf);
        void             DestroyTransform(const game_TransformId& id);
        void             GarbageCollect(const game_EntityManager& entityManager);

        bool             HasTransform(const game_Entity& entity) const;
        game_TransformId GetTransformId(const game_Entity& entity) const;

        //        void Translate(const game_TransformId& id, const math_Vec3& translation);
        void Rotate(const game_TransformId& id, const math_Vec3& axis, float degrees);
        //        void Scale(const game_TransformId& id, const math_Vec3& scale);

        //        void SetLocalPosition(const game_TransformId& id, const math_Vec3& position);
        //        void SetLocalRotation(const game_TransformId& id, const math_Quat& rotation);
        //        void SetLocalScale(const game_TransformId& id, const math_Vec3& scale);
        void SetLocal(const game_TransformId& id, const math_Mat4x4& matrix);

        //        math_Vec3   GetLocalPosition(const game_TransformId& id) const;
        //        math_Quat   GetLocalRotation(const game_TransformId& id) const;
        //        math_Vec3   GetLocalScale(const game_TransformId& id) const;
        math_Mat4x4 GetLocal(const game_TransformId& id) const;

        //        math_Vec3   GetWorldPosition(const game_TransformId& id) const;
        //        math_Quat   GetWorldRotation(const game_TransformId& id) const;
        //        math_Vec3   GetWorldScale(const game_TransformId& id) const;
        math_Mat4x4 GetWorld(const game_TransformId& id) const;

    private:
        void Transform(const game_TransformId& id, const math_Mat4x4& parent);
    };
} // namespace pge

#endif