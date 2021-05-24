#ifndef PGE_ANIMATION_ANIM_ANIMATOR_H
#define PGE_ANIMATION_ANIM_ANIMATOR_H

#include "anim_skeleton.h"
#include <string>
#include <vector>
#include <iostream>

namespace pge
{
    struct anim_AnimationState {
        std::string                   name;
        const anim_SkeletonAnimation* animation;
        bool                          looping;

        anim_AnimationState()
            : name("")
            , animation(nullptr)
            , looping(false)
        {}

        anim_AnimationState(const char* name, const anim_SkeletonAnimation* animation, bool looping)
            : name(name)
            , animation(animation)
            , looping(looping)
        {}
    };

    struct anim_TransitionParam {
        std::string from;
        std::string to;
        std::string trigger;
        float       duration;

        anim_TransitionParam() = default;
        anim_TransitionParam(const char* from, const char* to, const char* trigger, float duration)
            : from(from)
            , to(to)
            , trigger(trigger)
            , duration(duration)
        {}
    };

    class anim_AnimatorConfig {
        friend class anim_Animator;
        struct Transition {
            const anim_AnimationState* from;
            const anim_AnimationState* to;
            std::string                trigger;
            float                      duration;
        };
        const anim_Skeleton*             m_skeleton;
        std::vector<anim_AnimationState> m_states;
        std::vector<Transition>          m_transitions;

    public:
        void Initialize(const anim_Skeleton*        skeleton,
                        const anim_AnimationState*  states,
                        unsigned                    numStates,
                        const anim_TransitionParam* transitions,
                        unsigned                    numTransitions);

        anim_AnimatorConfig() = default;
        anim_AnimatorConfig(const anim_Skeleton*        skeleton,
                            const anim_AnimationState*  states,
                            unsigned                    numStates,
                            const anim_TransitionParam* transitions,
                            unsigned                    numTransitions);
    };

    class anim_Animator {
        const anim_AnimatorConfig*             m_config;
        const anim_AnimationState*             m_currentState;
        const anim_AnimatorConfig::Transition* m_currentTrans;
        float                                  m_currentAnimTime;
        float                                  m_transitTime;

    public:
        anim_Animator(const anim_AnimatorConfig* config);

        void          Trigger(const char* trigger);
        void          Update(float dt);
        anim_Skeleton GetAnimatedSkeleton() const;
    };
} // namespace pge

#endif
