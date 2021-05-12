#ifndef PGE_EDITOR_EDIT_ENTITY_H
#define PGE_EDITOR_EDIT_ENTITY_H

#include "edit_command.h"
#include <game_world.h>
#include <memory>

namespace pge
{
//    class edit_CommandSelectEntity : public edit_Command {
//        game_Entity  m_entity;
//        game_Entity  m_previous;
//        game_Entity* m_selected;
//
//    public:
//        edit_CommandSelectEntity(const game_Entity& entity, game_Entity* selected);
//
//        virtual void                         Do() override;
//        virtual void                         Undo() override;
//        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, game_Entity* selected);
//    };


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
        game_Entity                          GetCreatedEntity() const;
        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* scene, const game_Entity& entity);
    };
} // namespace pge

#endif