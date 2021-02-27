#ifndef PGE_GAME_GAME_ENTITY_H
#define PGE_GAME_GAME_ENTITY_H

#include <vector>
#include <unordered_map>

namespace pge
{
    using game_EntityId                              = unsigned;
    static const game_EntityId game_EntityId_Invalid = -1;
    struct game_Entity {
        unsigned id;

        game_Entity();
        game_Entity(game_EntityId id);
        game_Entity(unsigned index, unsigned generation);

        bool
        operator==(const game_Entity& other) const
        {
            return id == other.id;
        }

        unsigned GetIndex() const;
        unsigned GetGeneration() const;
    };

    class game_EntityManager {
        std::vector<unsigned> m_generation;
        std::vector<unsigned> m_freeIndices;

    public:
        game_EntityManager();
        game_EntityManager(const game_Entity* entities, size_t numEntities);

        game_Entity CreateEntity();
        void        CreateEntities(game_Entity* destBuf, size_t numEntities);
        void        DestroyEntity(const game_Entity& entity);
        bool        IsEntityAlive(const game_Entity& entity) const;

        friend std::ostream& operator<<(std::ostream& os, const game_EntityManager& em);
        friend std::istream& operator>>(std::istream& is, game_EntityManager& em);
    };
} // namespace pge

namespace std
{
    template <>
    class hash<pge::game_Entity> {
    public:
        inline size_t
        operator()(const pge::game_Entity& entity) const
        {
            return entity.id;
        }
    };
} // namespace std

// NOTE: This needs to be placed after the hash declaration above, because of the
//         use of game_Entity in the game_EntityDataManager's unordered_map.
namespace pge
{
    struct game_EntityMetaData {
        game_Entity entity;
        char        name[32];

        game_EntityMetaData()
            : entity(game_EntityId_Invalid)
        {}

        game_EntityMetaData(const game_Entity& entity, const char* name)
            : entity(entity)
        {
            strcpy_s(this->name, name);
        }
    };

    using game_EntityMetaDataIterator      = std::unordered_map<game_Entity, game_EntityMetaData>::iterator;
    using game_EntityMetaDataConstIterator = std::unordered_map<game_Entity, game_EntityMetaData>::const_iterator;
    class game_EntityMetaDataManager {
        std::unordered_map<game_Entity, game_EntityMetaData> m_entityMap;

    public:
        void                CreateMetaData(const game_Entity& entity, const game_EntityMetaData& data);
        void                DestroyMetaData(const game_Entity& entity);
        void                GarbageCollect(const game_EntityManager& manager);
        bool                HasMetaData(const game_Entity& entity) const;
        game_EntityMetaData GetMetaData(const game_Entity& entity) const;
        void                SetMetaData(const game_Entity& entity, const game_EntityMetaData& data);

        game_EntityMetaDataIterator      Begin();
        game_EntityMetaDataIterator      End();
        game_EntityMetaDataConstIterator CBegin() const;
        game_EntityMetaDataConstIterator CEnd() const;

        friend std::ostream& operator<<(std::ostream& os, const game_EntityMetaDataManager& mm);
        friend std::istream& operator>>(std::istream& is, game_EntityMetaDataManager& mm);
    };
} // namespace pge

#endif