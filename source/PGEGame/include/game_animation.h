#ifndef PGE_GAME_GAME_ANIMATION_H
#define PGE_GAME_GAME_ANIMATION_H

#include "game_entity.h"
#include <res_animator.h>
#include <vector>
#include <unordered_map>

namespace pge
{
    class game_AnimationManager {
        std::vector<game_Entity>                m_entities;
        std::vector<anim_Animator>              m_animators;
        std::unordered_map<game_Entity, size_t> m_entityMap;
        std::vector<size_t>                     m_freeList;

    public:
        game_AnimationManager(size_t capacity);
        ~game_AnimationManager();

        void CreateAnimator(const game_Entity& entity, const res_AnimatorConfig* config);
        void DestroyAnimator(const game_Entity& entity);
        void GarbageCollect(const game_EntityManager& entityManager);

        bool HasAnimator(const game_Entity& entity) const;
        void SetAnimator(const game_Entity& entity, const res_AnimatorConfig* config);

        anim_Skeleton GetAnimatedSkeleton(const game_Entity& entity) const;
        void          Update(float dt);
        void          Trigger(const char* trigger);
        void          Trigger(const game_Entity& entity, const char* trigger);
    };
} // namespace pge

#endif