#ifndef PGE_GAME_GAME_ANIMATION_H
#define PGE_GAME_GAME_ANIMATION_H

#include "game_entity.h"
#include <res_animator.h>

namespace pge
{
//    using game_AnimatorId                         = unsigned;
//    static const unsigned game_AnimatorId_Invalid = -1;
//    class game_AnimationManager {
//        struct Instance {
//            const res_AnimatorConfig*    animConfig;
//            anim_Animator                animator;
//        };
//
//    public:
//        explicit game_AnimationManager(size_t capacity);
//        ~game_AnimationManager();
//
//        game_AnimatorId CreateAnimator(const game_Entity& entity, const res_AnimatorConfig* config);
//        void            DestroyAnimator(const game_AnimatorId& id);
//        void            GarbageCollect(const game_EntityManager& entityManager);
//
//        bool             HasAnimator(const game_Entity& entity) const;
//        game_TransformId GetAnimatorId(const game_Entity& entity) const;
//
//        void Trigger(const char* trigger);
//        void Trigger(const game_Animator)
//    };
} // namespace pge

#endif