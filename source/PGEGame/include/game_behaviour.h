#ifndef PGE_GAME_GAME_BEHAVIOUR_H
#define PGE_GAME_GAME_BEHAVIOUR_H

#include "game_entity.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace pge
{
    class game_Behaviour {
    public:
        virtual ~game_Behaviour()        = default;
        virtual void Update(float delta) = 0;
    };

    class game_BehaviourManager {
        std::unordered_map<game_Entity, std::unique_ptr<game_Behaviour>> m_behaviours;

    public:
        game_BehaviourManager();
        void CreateBehaviour(const game_Entity& entity, std::unique_ptr<game_Behaviour> behaviour);
        void DestroyBehaviour(const game_Entity& entity);
        void GarbageCollect(const game_EntityManager& entityManager);

        void Update(float delta);
    };
} // namespace pge

#endif