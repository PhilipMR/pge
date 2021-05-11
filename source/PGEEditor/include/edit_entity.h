#ifndef PGE_EDITOR_EDIT_ENTITY_H
#define PGE_EDITOR_EDIT_ENTITY_H

#include "edit_command.h"
#include <memory>
#include <game_world.h>

namespace pge
{
    class edit_CommandSelectEntity : public edit_Command {
        game_Entity  m_entity;
        game_Entity  m_previous;
        game_Entity* m_selected;

    public:
        edit_CommandSelectEntity(const game_Entity& entity, game_Entity* selected);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, game_Entity* selected);
    };


    class edit_CommandDeleteEntity : public edit_Command {
        game_Entity           m_entity;
        game_World*           m_world;
        game_SerializedEntity m_sentity;

    public:
        edit_CommandDeleteEntity(const game_Entity& entity, game_World* world);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, game_World* world);
    };


    class edit_CommandCreateEntity : public edit_Command {
        game_World*           m_world;
        game_Entity           m_createdEntity;
        game_SerializedEntity m_sentity;

    public:
        edit_CommandCreateEntity(game_World* world);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* world);
    };


    class edit_CommandDuplicateEntity : public edit_Command {
        game_World*           m_world;
        game_SerializedEntity m_sentity;
        game_Entity           m_duplicate;

    public:
        edit_CommandDuplicateEntity(game_World* world, const game_Entity& entity);
        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* scene, const game_Entity& entity);
    };


    class edit_CommandCreateDirectionalLight : public edit_Command {
        game_EntityManager*         m_entityManager;
        game_EntityMetaDataManager* m_metaManager;
        game_TransformManager*      m_transformManager;
        game_LightManager*          m_lightManager;
        game_Entity                 m_createdEntity;

    public:
        edit_CommandCreateDirectionalLight(game_EntityManager*         emanager,
                                           game_EntityMetaDataManager* metaManager,
                                           game_TransformManager*      tmanager,
                                           game_LightManager*          lmanager);

        virtual void Do() override;
        virtual void Undo() override;
        static std::unique_ptr<edit_Command>
        Create(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager, game_TransformManager* tmanager, game_LightManager* lmanager);
    };


    class edit_CommandCreatePointLight : public edit_Command {
        game_EntityManager*         m_entityManager;
        game_EntityMetaDataManager* m_metaManager;
        game_TransformManager*      m_transformManager;
        game_LightManager*          m_lightManager;
        game_Entity                 m_createdEntity;

    public:
        edit_CommandCreatePointLight(game_EntityManager*         emanager,
                                     game_EntityMetaDataManager* metaManager,
                                     game_TransformManager*      tmanager,
                                     game_LightManager*          lmanager);

        virtual void Do() override;
        virtual void Undo() override;
        static std::unique_ptr<edit_Command>
        Create(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager, game_TransformManager* tmanager, game_LightManager* lmanager);
    };
} // namespace pge

#endif