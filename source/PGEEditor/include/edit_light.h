#ifndef PGE_EDITOR_EDIT_LIGHT_H
#define PGE_EDITOR_EDIT_LIGHT_H

#include "edit_component.h"
#include "edit_command.h"
#include <game_world.h>

namespace pge
{
    class edit_LightEditor : public edit_ComponentEditor {
        game_LightManager* m_lightManager;

    public:
        edit_LightEditor(game_LightManager* lm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
    };


    class edit_CommandCreateDirectionalLight : public edit_Command {
        game_World*           m_world;
        game_Entity           m_createdEntity;
        game_SerializedEntity m_sentity;

    public:
        edit_CommandCreateDirectionalLight(game_World* world);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* world);
    };

    class edit_CommandCreatePointLight : public edit_Command {
        game_World*           m_world;
        game_Entity           m_createdEntity;
        game_SerializedEntity m_sentity;

    public:
        edit_CommandCreatePointLight(game_World* world);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_World* world);
    };
} // namespace pge

#endif