#ifndef PGE_EDITOR_EDIT_ENTITY_H
#define PGE_EDITOR_EDIT_ENTITY_H

#include "edit_command.h"
#include <memory>
#include <game_scene.h>

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
        game_Entity m_entity;
        game_Scene* m_scene;

        bool                m_hasMetaData;
        game_EntityMetaData m_metaData;

        bool      m_hasTransform;
        math_Vec3 m_localPos;
        math_Quat m_localRot;
        math_Vec3 m_localScale;

        bool            m_hasPointLight;
        game_PointLight m_pointLight;

        bool                m_hasMesh;
        const res_Material* m_meshMaterial;
        const res_Mesh*     m_meshMesh;

    public:
        edit_CommandDeleteEntity(const game_Entity& entity, game_Scene* scene);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(const game_Entity& entity, game_Scene* scene);
    };

    class edit_CommandCreateEntity : public edit_Command {
        game_EntityManager*         m_entityManager;
        game_EntityMetaDataManager* m_metaManager;
        game_Entity                 m_createdEntity;

    public:
        edit_CommandCreateEntity(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager);

        virtual void                         Do() override;
        virtual void                         Undo() override;
        static std::unique_ptr<edit_Command> Create(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager);
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