#include "../include/game_behaviour.h"

namespace pge
{
    game_BehaviourManager::game_BehaviourManager() {}

    void
    game_BehaviourManager::CreateBehaviour(const game_Entity& entity, std::unique_ptr<game_Behaviour> behaviour)
    {
        m_behaviours[entity] = std::move(behaviour);
    }

    void
    game_BehaviourManager::DestroyBehaviour(const game_Entity& entity)
    {
        m_behaviours.erase(m_behaviours.find(entity));
    }

    void
    game_BehaviourManager::GarbageCollect(const game_EntityManager& entityManager)
    {}

    void
    game_BehaviourManager::Update(float delta)
    {
        for (auto& beh : m_behaviours) {
            beh.second->Update(delta);
        }
    }
} // namespace pge