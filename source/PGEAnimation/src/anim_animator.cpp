#include "../include/anim_animator.h"

#include <core_assert.h>

namespace pge
{
    void
    anim_AnimatorConfig::Initialize(const anim_Skeleton*        skeleton,
                                    const anim_AnimationState*  states,
                                    unsigned int                numStates,
                                    const anim_TransitionParam* transitions,
                                    unsigned int                numTransitions)
    {
        m_states.clear();
        m_transitions.clear();
        m_skeleton = skeleton;

        m_states.reserve(numStates);
        for (unsigned i = 0; i < numStates; ++i) {
            m_states.emplace_back(states[i]);
        }

        m_transitions.reserve(numTransitions);
        for (unsigned i = 0; i < numTransitions; ++i) {
            const anim_TransitionParam& transParam = transitions[i];

            auto fromIt
                = std::find_if(m_states.begin(), m_states.end(), [&](const anim_AnimationState& state) { return state.name == transParam.from; });
            core_Assert(fromIt != m_states.end());

            auto toIt = std::find_if(m_states.begin(), m_states.end(), [&](const anim_AnimationState& state) { return state.name == transParam.to; });
            core_Assert(toIt != m_states.end());

            Transition transition;
            transition.from     = &*fromIt;
            transition.to       = &*toIt;
            transition.duration = transParam.duration;
            transition.trigger  = transParam.trigger;
            m_transitions.emplace_back(transition);
        }
    }

    anim_AnimatorConfig::anim_AnimatorConfig(const anim_Skeleton*        skeleton,
                                             const anim_AnimationState*  states,
                                             unsigned                    numStates,
                                             const anim_TransitionParam* transitions,
                                             unsigned                    numTransitions)
    {
        Initialize(skeleton, states, numStates, transitions, numTransitions);
    }

    anim_Animator::anim_Animator(const anim_AnimatorConfig* config)
        : m_config(config)
        , m_currentTrans(nullptr)
        , m_currentAnimTime(0)
        , m_transitTime(0)
    {
        core_Assert(config->m_states.size() > 0);
        m_currentState = &config->m_states[0];
    }

    void
    anim_Animator::Trigger(const char* trigger)
    {
        for (const auto& transition : m_config->m_transitions) {
            if (transition.from != m_currentState)
                continue;
            if (transition.trigger == trigger) {
                m_currentTrans = &transition;
                m_transitTime  = 0;
            }
        }
    }

    void
    anim_Animator::Update(float dt)
    {
        if (m_currentTrans == nullptr) {
            m_currentAnimTime += dt;
            if (m_currentState->looping && m_currentAnimTime >= m_currentState->animation->GetDuration()) {
                m_currentAnimTime = 0;
            }
        } else {
            m_currentAnimTime += dt;
            m_transitTime += dt;
            if (m_transitTime >= m_currentTrans->duration) {
                m_currentState    = m_currentTrans->to;
                m_currentAnimTime = 0;
                m_currentTrans    = nullptr;
                m_transitTime     = 0;
            }
        }
    }

    anim_Skeleton
    anim_Animator::GetAnimatedSkeleton() const
    {
        anim_Skeleton animatedSkel = *m_config->m_skeleton;
        if (m_currentTrans == nullptr) {
            animatedSkel.Animate(*m_currentState->animation, m_currentAnimTime);
        } else {
            float factor = m_transitTime / m_currentTrans->duration;
            animatedSkel.Animate(*m_currentTrans->from->animation, m_currentAnimTime, *m_currentTrans->to->animation, 0, factor);
        }
        animatedSkel.Transform();
        return animatedSkel;
    }
} // namespace pge