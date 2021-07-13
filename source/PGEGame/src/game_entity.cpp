#include "../include/game_entity.h"
#include <core_assert.h>
#include <iostream>
#include <sstream>

namespace pge
{
    static const unsigned EntityIndexBits = 22;
    static const unsigned EntityIndexMask = (1 << EntityIndexBits) - 1;

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
    // game_EntityIterator
    // -------------------------------------------------------
    game_EntityManager::game_EntityIterator::game_EntityIterator(const game_EntityManager* manager, unsigned idx)
        : m_manager(manager)
        , m_idx(idx)
    {}

    const game_Entity
    game_EntityManager::game_EntityIterator::operator*() const
    {
        return game_Entity(m_idx, m_manager->m_generation.at(m_idx));
    }

    // Prefix increment
    game_EntityManager::game_EntityIterator&
    game_EntityManager::game_EntityIterator::operator++()
    {
        auto nextIdx = m_idx + 1;
        while (std::find(m_manager->m_freeIndices.begin(), m_manager->m_freeIndices.end(), nextIdx) != m_manager->m_freeIndices.end()) {
            nextIdx++;
        }
        m_idx = nextIdx;
        return *this;
    }

    // Postfix increment
    game_EntityManager::game_EntityIterator
    game_EntityManager::game_EntityIterator::operator++(int)
    {
        game_EntityIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool
    operator==(const game_EntityManager::game_EntityIterator& a, const game_EntityManager::game_EntityIterator& b)
    {
        return a.m_idx == b.m_idx;
    };

    bool
    operator!=(const game_EntityManager::game_EntityIterator& a, const game_EntityManager::game_EntityIterator& b)
    {
        return a.m_idx != b.m_idx;
    };


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
            core_Assert(idx < (1 << EntityIndexBits));
        }
        return game_Entity(idx, m_generation[idx]);
    }

    game_Entity
    game_EntityManager::CreateEntity(const char* name)
    {
        game_Entity entity = CreateEntity();
        SetName(entity, name);
        return entity;
    }

    void
    game_EntityManager::CreateEntity(const game_Entity& entity)
    {
        core_Assert(!IsEntityAlive(entity));
        auto it = std::find(m_freeIndices.begin(), m_freeIndices.end(), entity.GetIndex());
        if (it != m_freeIndices.end()) {
            m_freeIndices.erase(it);
        }
        m_generation[entity.GetIndex()] = entity.GetGeneration();
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

        auto it = m_names.find(entity);
        if (it != m_names.end()) {
            m_names.erase(it);
        }
    }

    bool
    game_EntityManager::IsEntityAlive(const game_Entity& entity) const
    {
        return m_generation[entity.GetIndex()] == entity.GetGeneration();
    }

    std::string
    game_EntityManager::GetName(const game_Entity& entity) const
    {
        core_Assert(IsEntityAlive(entity));
        auto it = m_names.find(entity);
        if (it == m_names.end()) {
            std::stringstream ss;
            ss << "Entity [" << entity.id << "]";
            return ss.str();
        } else {
            return it->second;
        }
    }

    void
    game_EntityManager::SetName(const game_Entity& entity, const char* name)
    {
        core_Assert(IsEntityAlive(entity));
        m_names[entity] = name;
    }

    game_EntityManager::game_EntityIterator
    game_EntityManager::begin() const
    {
        unsigned idx = 0;
        while (std::find(m_freeIndices.begin(), m_freeIndices.end(), idx) != m_freeIndices.end()) {
            ++idx;
        }
        return game_EntityIterator(this, idx);
    }

    game_EntityManager::game_EntityIterator
    game_EntityManager::end() const
    {
        return game_EntityIterator(this, m_generation.size());
    }

    void
    game_EntityManager::SerializeEntity(std::ostream& os, const game_Entity& entity) const
    {
        core_Assert(IsEntityAlive(entity));
        const std::string& name    = m_names.at(entity);
        unsigned           nameLen = name.size();
        os.write((const char*)&nameLen, sizeof(nameLen));
        os.write(name.c_str(), nameLen);
    }

    void
    game_EntityManager::InsertSerializedEntity(std::istream& is, const game_Entity& entity)
    {
        unsigned nameLen;
        is.read((char*)&nameLen, sizeof(nameLen));
        std::unique_ptr<char[]> name(new char[nameLen+1]);
        is.read(name.get(), nameLen);
        name[nameLen] = 0;
        if (!IsEntityAlive(entity)) {
            CreateEntity(entity);
        }
        SetName(entity, name.get());
    }

    std::ostream&
    operator<<(std::ostream& os, const game_EntityManager& em)
    {
        unsigned numEntities = em.m_generation.size() - em.m_freeIndices.size();
        os.write((const char*)&numEntities, sizeof(numEntities));
        for (size_t i = 0; i < em.m_generation.size(); ++i) {
            // If i is in m_freeIndices, then that entity was deleted
            if (std::find(em.m_freeIndices.begin(), em.m_freeIndices.end(), i) != em.m_freeIndices.end())
                continue;
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
        if (numEntities == 0)
            return is;

        em.m_generation.resize(numEntities);
        for (size_t i = 0; i < numEntities; ++i) {
            game_EntityId entityId = 0;
            is.read((char*)&entityId, sizeof(entityId));
            game_Entity entity(entityId);
            while (entity.GetIndex() >= em.m_generation.size())
                em.m_generation.push_back(0);
            em.m_generation[entity.GetIndex()] = entity.GetGeneration();
        }
        return is;
    }
} // namespace pge