#include "../include/game_animation.h"

namespace pge
{
    game_AnimationManager::game_AnimationManager(size_t capacity)
    {
        m_animators.reserve(capacity);
        m_entities.reserve(capacity);
    }

    game_AnimationManager::~game_AnimationManager() {}

    void
    game_AnimationManager::CreateAnimator(const game_Entity& entity, const res_AnimatorConfig* config)
    {
        if (!m_freeList.empty()) {
            size_t idx = m_freeList.back();
            m_freeList.pop_back();
            m_entityMap[entity] = idx;
            m_entities[idx]     = entity;
            m_animators[idx]    = anim_Animator(config->GetConfig());
        } else {
            m_entities.push_back(entity);
            m_animators.push_back(anim_Animator(config->GetConfig()));
            size_t idx          = m_animators.size() - 1;
            m_entityMap[entity] = idx;
        }
    }

    void
    game_AnimationManager::DestroyAnimator(const game_Entity& entity)
    {
        core_Assert(HasAnimator(entity));
        size_t idx = m_entityMap.at(entity);
        m_entityMap.erase(m_entityMap.find(entity));
        m_freeList.push_back(idx);
    }

    void
    game_AnimationManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (size_t aliveStreak = 0; m_entityMap.size() > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_entityMap.size();
            if (!entityManager.IsEntityAlive(m_entities[randIdx])) {
                DestroyAnimator(randIdx);
                aliveStreak = 0;
            } else {
                aliveStreak++;
            }
        }
    }

    bool
    game_AnimationManager::HasAnimator(const game_Entity& entity) const
    {
        return m_entityMap.find(entity) != m_entityMap.end();
    }

    void
    game_AnimationManager::SetAnimator(const game_Entity& entity, const res_AnimatorConfig* config)
    {
        if (!HasAnimator(entity)) {
            CreateAnimator(entity, config);
        } else {
            size_t idx       = m_entityMap.at(entity);
            m_animators[idx] = anim_Animator(config->GetConfig());
        }
    }

    anim_Skeleton
    game_AnimationManager::GetAnimatedSkeleton(const game_Entity& entity) const
    {
        core_Assert(HasAnimator(entity));
        size_t idx = m_entityMap.at(entity);
        return m_animators.at(idx).GetAnimatedSkeleton();
    }

    void
    game_AnimationManager::Update(float dt)
    {
        for (auto& animator : m_animators) {
            animator.Update(dt);
        }
    }

    void
    game_AnimationManager::Trigger(const char* trigger)
    {
        for (auto& animator : m_animators) {
            animator.Trigger(trigger);
        }
    }

    void
    game_AnimationManager::Trigger(const game_Entity& entity, const char* trigger)
    {
        core_Assert(HasAnimator(entity));
        size_t idx = m_entityMap.at(entity);
        m_animators.at(idx).Trigger(trigger);
    }
} // namespace pge