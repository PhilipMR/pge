#include "../include/game_entity.h"
#include <diag_assert.h>
#include <iostream>

namespace pge
{
    static const unsigned EntityIndexBits   = 22;
    static const unsigned EntityIndexMask   = (1 << EntityIndexBits) - 1;

    static const unsigned EntityGenerationBits = 10;
    static const unsigned EntityGenerationMask = (1 << EntityGenerationBits) - 1;

    static const unsigned MinimumFreeIndices = 1024;

    // -------------------------------------------------------
    // game_Entity
    // -------------------------------------------------------
    game_Entity::game_Entity()
        : id(game_EntityId_Invalid)
    {}

    game_Entity::game_Entity(game_EntityId id)
        : id(id)
    {}

    game_Entity::game_Entity(unsigned index, unsigned generation)
        : id(index | (generation << EntityIndexBits))
    {}

    unsigned
    game_Entity::GetIndex() const
    {
        return id & EntityIndexMask;
    }

    unsigned
    game_Entity::GetGeneration() const
    {
        return (id >> EntityIndexBits) & EntityGenerationMask;
    }


    // -------------------------------------------------------
    // game_EntityManager
    // -------------------------------------------------------
    game_EntityManager::game_EntityManager() = default;
    game_EntityManager::game_EntityManager(const game_Entity* entities, size_t numEntities)
    {
        m_generation.reserve(numEntities);
        for (size_t i = 0; i < numEntities; ++i) {
            const game_Entity& entity       = entities[i];
            m_generation[entity.GetIndex()] = entity.GetGeneration();
        }
    }

    game_Entity
    game_EntityManager::CreateEntity()
    {
        unsigned idx;
        if (m_freeIndices.size() > MinimumFreeIndices) {
            idx = m_freeIndices.back();
            m_freeIndices.pop_back();
        } else {
            m_generation.push_back(0);
            idx = m_generation.size() - 1;
            diag_Assert(idx < (1 << EntityIndexBits));
        }
        return game_Entity(idx, m_generation[idx]);
    }

    void
    game_EntityManager::CreateEntities(game_Entity* destBuf, size_t numEntities)
    {
        for (size_t i = 0; i < numEntities; ++i) {
            destBuf[i] = CreateEntity();
        }
    }

    void
    game_EntityManager::DestroyEntity(const game_Entity& entity)
    {
        unsigned idx = entity.GetIndex();
        m_generation[idx]++;
        m_freeIndices.push_back(idx);
    }

    bool
    game_EntityManager::IsEntityAlive(const game_Entity& entity) const
    {
        return m_generation[entity.GetIndex()] == entity.GetGeneration();
    }

    std::ostream&
    operator<<(std::ostream& os, const game_EntityManager& em)
    {
        unsigned numEntities = em.m_generation.size();
        os.write((const char*)&numEntities, sizeof(numEntities));
        for (size_t i = 0; i < em.m_generation.size(); ++i) {
            const game_Entity entity(i, em.m_generation[i]);
            os.write((const char*)&entity.id, sizeof(game_EntityId));
        }
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_EntityManager& em)
    {
        em.m_generation.clear();
        em.m_freeIndices.clear();
        unsigned numEntities = 0;
        is.read((char*)&numEntities, sizeof(numEntities));
        diag_Assert(numEntities > 0);
        em.m_generation.resize(numEntities);
        for (size_t i = 0; i < numEntities; ++i) {
            game_EntityId entityId = 0;
            is.read((char*)&entityId, sizeof(entityId));
            game_Entity entity(entityId);
            em.m_generation[entity.GetIndex()] = entity.GetGeneration();
        }
        return is;
    }

    // -------------------------------------------------------
    // game_EntityMetaDataManager
    // -------------------------------------------------------
    void
    game_EntityMetaDataManager::CreateMetaData(const game_Entity& entity, const game_EntityMetaData& data)
    {
        diag_Assert(!HasMetaData(entity));
        m_entityMap.insert(std::make_pair(entity, data));
    }

    void
    game_EntityMetaDataManager::DestroyMetaData(const game_Entity& entity)
    {
        diag_Assert(HasMetaData(entity));
        m_entityMap.erase(entity);
    }

    void
    game_EntityMetaDataManager::GarbageCollect(const game_EntityManager& manager)
    {
        size_t                   aliveStreak = 0;
        std::vector<game_Entity> entitiesToRemove;
        for (const auto& kv : m_entityMap) {
            if (manager.IsEntityAlive(kv.first)) {
                aliveStreak++;
            } else {
                entitiesToRemove.push_back(kv.first);
                aliveStreak = 0;
            }
            if (aliveStreak >= 4)
                return;
        }
        for (const auto& e : entitiesToRemove) {
            m_entityMap.erase(e);
        }
    }

    bool
    game_EntityMetaDataManager::HasMetaData(const game_Entity& entity) const
    {
        return m_entityMap.find(entity) != m_entityMap.end();
    }

    game_EntityMetaData
    game_EntityMetaDataManager::GetMetaData(const game_Entity& entity) const
    {
        diag_Assert(HasMetaData(entity));
        return m_entityMap.find(entity)->second;
    }

    void
    game_EntityMetaDataManager::SetMetaData(const game_Entity& entity, const game_EntityMetaData& data)
    {
        diag_Assert(HasMetaData(entity));
        m_entityMap.find(entity)->second = data;
    }

    game_EntityMetaDataIterator
    game_EntityMetaDataManager::Begin()
    {
        return m_entityMap.begin();
    }

    game_EntityMetaDataIterator
    game_EntityMetaDataManager::End()
    {
        return m_entityMap.end();
    }

    game_EntityMetaDataConstIterator
    game_EntityMetaDataManager::CBegin() const
    {
        return m_entityMap.cbegin();
    }

    game_EntityMetaDataConstIterator
    game_EntityMetaDataManager::CEnd() const
    {
        return m_entityMap.cend();
    }

    std::ostream&
    operator<<(std::ostream& os, const game_EntityMetaDataManager& mm)
    {
        unsigned numComponents = mm.m_entityMap.size();
        os.write((const char*)&numComponents, sizeof(numComponents));
        for (const auto& kv : mm.m_entityMap) {
            os.write((const char*)&kv.second, sizeof(kv.second));
        }
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_EntityMetaDataManager& mm)
    {
        mm.m_entityMap.clear();
        unsigned numComponents = 0;
        is.read((char*)&numComponents, sizeof(numComponents));
        for (unsigned i = 0; i < numComponents; ++i) {
            game_EntityMetaData meta;
            is.read((char*)&meta, sizeof(meta));
            mm.m_entityMap.insert(std::make_pair<>(meta.entity, meta));
        }
        return is;
    }
} // namespace pge