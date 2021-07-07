#ifndef PGE_GAME_GAME_ENTITY_H
#define PGE_GAME_GAME_ENTITY_H

#include <vector>
#include <unordered_map>
#include <string>
#include <iterator>

namespace pge
{
    using game_EntityId                              = unsigned;
    static const game_EntityId game_EntityId_Invalid = -1;
    struct game_Entity {
        unsigned id;

        game_Entity();
        game_Entity(game_EntityId id);
        game_Entity(unsigned index, unsigned generation);

        unsigned GetIndex() const;
        unsigned GetGeneration() const;
    };

    inline bool
    operator==(const game_Entity& lhs, const game_Entity& rhs)
    {
        return lhs.id == rhs.id;
    }

    inline bool
    operator!=(const game_Entity& lhs, const game_Entity& rhs)
    {
        return lhs.id != rhs.id;
    }
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

namespace pge
{
    class game_EntityManager {
        std::vector<unsigned>                        m_generation;
        std::vector<unsigned>                        m_freeIndices;
        std::unordered_map<game_Entity, std::string> m_names;

    public:
        class game_EntityIterator : public std::iterator<std::forward_iterator_tag, game_Entity> {
            const game_EntityManager* m_manager;
            unsigned                  m_idx;

        public:
            game_EntityIterator(const game_EntityManager* manager, unsigned idx);
            const game_Entity operator*() const;

            game_EntityIterator& operator++();    // Prefix increment
            game_EntityIterator  operator++(int); // Postfix increment

            friend bool operator==(const game_EntityIterator& a, const game_EntityIterator& b);
            friend bool operator!=(const game_EntityIterator& a, const game_EntityIterator& b);
        };

        game_EntityManager();
        game_EntityManager(const game_Entity* entities, size_t numEntities);

        game_Entity CreateEntity();
        game_Entity CreateEntity(const char* name);
        void        CreateEntity(const game_Entity& entity);
        void        CreateEntities(game_Entity* destBuf, size_t numEntities);
        void        DestroyEntity(const game_Entity& entity);
        bool        IsEntityAlive(const game_Entity& entity) const;

        std::string GetName(const game_Entity& entity) const;
        void        SetName(const game_Entity& entity, const char* name);

        game_EntityIterator begin() const;
        game_EntityIterator end() const;

        void SerializeEntity(std::ostream& os, const game_Entity& entity) const;
        void InsertSerializedEntity(std::istream& is, const game_Entity& entity);

        friend std::ostream& operator<<(std::ostream& os, const game_EntityManager& em);
        friend std::istream& operator>>(std::istream& is, game_EntityManager& em);
    };


} // namespace pge

#endif