#include "../include/edit_entity.h"
#include <sstream>

namespace pge
{
    // ===============================
    // edit_CommandSelectEntity
    // ===============================
//    edit_CommandSelectEntity::edit_CommandSelectEntity(const game_Entity& entity, game_Entity* selected)
//        : m_entity(entity)
//        , m_previous(game_EntityId_Invalid)
//        , m_selected(selected)
//    {}
//
//    void
//    edit_CommandSelectEntity::Do()
//    {
//        m_previous  = *m_selected;
//        *m_selected = m_entity;
//    }
//
//    void
//    edit_CommandSelectEntity::Undo()
//    {
//        *m_selected = m_previous;
//    }
//
//    std::unique_ptr<edit_Command>
//    edit_CommandSelectEntity::Create(const game_Entity& entity, game_Entity* selected)
//    {
//        return std::unique_ptr<edit_Command>(new edit_CommandSelectEntity(entity, selected));
//    }


    // ===============================
    // edit_CommandDeleteEntity
    // ===============================
    edit_CommandDeleteEntity::edit_CommandDeleteEntity(const game_Entity& entity, game_World* world)
        : m_entity(entity)
        , m_world(world)
        , m_sentity(world->SerializeEntity(entity))
    {}

    void
    edit_CommandDeleteEntity::Do()
    {
        core_Assert(m_world->GetEntityManager()->IsEntityAlive(m_entity));
        m_world->GetEntityManager()->DestroyEntity(m_entity);
    }

    void
    edit_CommandDeleteEntity::Undo()
    {
        core_Assert(!m_world->GetEntityManager()->IsEntityAlive(m_entity));
        m_world->InsertSerializedEntity(m_sentity, m_entity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandDeleteEntity::Create(const game_Entity& entity, game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandDeleteEntity(entity, world));
    }


    // ===============================
    // edit_CommandCreateEntity
    // ===============================
    edit_CommandCreateEntity::edit_CommandCreateEntity(game_World* world)
        : m_world(world)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreateEntity::Do()
    {
        if (m_createdEntity == game_EntityId_Invalid) {
            m_createdEntity = m_world->GetEntityManager()->CreateEntity();

            game_EntityMetaData meta;
            meta.entity = m_createdEntity;
            std::stringstream ss;
            ss << "Entity [" << m_createdEntity.id << "]";
            strcpy_s(meta.name, ss.str().c_str());
            m_world->GetEntityMetaDataManager()->CreateMetaData(m_createdEntity, meta);
        } else {
            m_world->InsertSerializedEntity(m_sentity, m_createdEntity);
        }
    }

    void
    edit_CommandCreateEntity::Undo()
    {
        core_Assert(m_createdEntity != game_EntityId_Invalid);
        m_sentity = m_world->SerializeEntity(m_createdEntity);
        m_world->GetEntityManager()->DestroyEntity(m_createdEntity);
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateEntity::Create(game_World* world)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateEntity(world));
    }


    // ===============================
    // edit_CommandDuplicateEntity
    // ===============================
    edit_CommandDuplicateEntity::edit_CommandDuplicateEntity(game_World* world, const game_Entity& entity)
        : m_world(world)
        , m_sentity(world->SerializeEntity(entity))
        , m_duplicate(game_EntityId_Invalid)
    {}

    game_Entity
    edit_CommandDuplicateEntity::GetCreatedEntity() const
    {
        return m_duplicate;
    }

    void
    edit_CommandDuplicateEntity::Do()
    {
        if (m_duplicate == game_EntityId_Invalid) {
            m_duplicate = m_world->InsertSerializedEntity(m_sentity);
        } else {
            m_world->InsertSerializedEntity(m_sentity, m_duplicate);
        }
    }

    void
    edit_CommandDuplicateEntity::Undo()
    {
        core_Assert(m_duplicate != game_EntityId_Invalid);
        m_world->GetEntityManager()->DestroyEntity(m_duplicate);
    }

    std::unique_ptr<edit_Command>
    edit_CommandDuplicateEntity::Create(game_World* scene, const game_Entity& entity)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandDuplicateEntity(scene, entity));
    }
} // namespace pge