#ifndef PGE_EDITOR_EDIT_LIGHT_H
#define PGE_EDITOR_EDIT_LIGHT_H

#include "edit_component.h"
#include "edit_command.h"
#include <game_light.h>

namespace pge
{
    class edit_LightEditor : public edit_ComponentEditor {
        game_LightManager* m_lightManager;

    public:
        edit_LightEditor(game_LightManager* lm);
        virtual void UpdateAndDraw(const game_Entity& entity) override;
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
}

#endif