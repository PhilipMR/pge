#include "../include/edit_entity.h"
#include <sstream>

namespace pge
{
    // ===============================
    // edit_CommandSelectEntity
    // ===============================
    edit_CommandSelectEntity::edit_CommandSelectEntity(const game_Entity& entity, game_Entity* selected)
        : m_entity(entity)
        , m_previous(game_EntityId_Invalid)
        , m_selected(selected)
    {}

    void
    edit_CommandSelectEntity::Do()
    {
        m_previous  = *m_selected;
        *m_selected = m_entity;
    }

    void
    edit_CommandSelectEntity::Undo()
    {
        *m_selected = m_previous;
    }

    std::unique_ptr<edit_Command>
    edit_CommandSelectEntity::Create(const game_Entity& entity, game_Entity* selected)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandSelectEntity(entity, selected));
    }


    // ===============================
    // edit_CommandDeleteEntity
    // ===============================
    edit_CommandDeleteEntity::edit_CommandDeleteEntity(const game_Entity& entity, game_Scene* scene)
        : m_entity(entity)
        , m_scene(scene)
    {}

    void
    edit_CommandDeleteEntity::Do()
    {
        diag_Assert(m_scene->GetEntityManager()->IsEntityAlive(m_entity));

        m_hasMetaData = m_scene->GetEntityMetaDataManager()->HasMetaData(m_entity);
        if (m_hasMetaData) {
            m_metaData = m_scene->GetEntityMetaDataManager()->GetMetaData(m_entity);
            m_scene->GetEntityMetaDataManager()->DestroyMetaData(m_entity);
        }

        m_hasTransform = m_scene->GetTransformManager()->HasTransform(m_entity);
        if (m_hasTransform) {
            game_TransformId tid = m_scene->GetTransformManager()->GetTransformId(m_entity);
            m_localPos           = m_scene->GetTransformManager()->GetLocalPosition(tid);
            m_localRot           = m_scene->GetTransformManager()->GetLocalRotation(tid);
            m_localScale         = m_scene->GetTransformManager()->GetLocalScale(tid);
            m_scene->GetTransformManager()->DestroyTransform(tid);
        }

        m_hasPointLight = m_scene->GetLightManager()->HasPointLight(m_entity);
        if (m_hasPointLight) {
            game_PointLightId pid = m_scene->GetLightManager()->GetPointLightId(m_entity);
            m_pointLight          = m_scene->GetLightManager()->GetPointLight(pid);
            m_scene->GetLightManager()->DestroyPointLight(pid);
        }

        m_hasMesh = m_scene->GetStaticMeshManager()->HasStaticMesh(m_entity);
        if (m_hasMesh) {
            game_StaticMeshId mid = m_scene->GetStaticMeshManager()->GetStaticMeshId(m_entity);
            m_meshMaterial        = m_scene->GetStaticMeshManager()->GetMaterial(mid);
            m_meshMesh            = m_scene->GetStaticMeshManager()->GetMesh(mid);
            m_scene->GetStaticMeshManager()->DestroyStaticMesh(mid);
        }

        m_scene->GetEntityManager()->DestroyEntity(m_entity);
    }

    void
    edit_CommandDeleteEntity::Undo()
    {
        diag_Assert(!m_scene->GetEntityManager()->IsEntityAlive(m_entity));
        m_scene->GetEntityManager()->CreateEntity(m_entity);
        if (m_hasMetaData) {
            m_scene->GetEntityMetaDataManager()->CreateMetaData(m_entity, m_metaData);
        }
        if (m_hasTransform) {
            m_scene->GetTransformManager()->CreateTransform(m_entity, m_localPos, m_localRot, m_localScale);
        }
        if (m_hasPointLight) {
            m_scene->GetLightManager()->CreatePointLight(m_entity, m_pointLight);
        }
        if (m_hasMesh) {
            game_StaticMeshId mid = m_scene->GetStaticMeshManager()->CreateStaticMesh(m_entity);
            m_scene->GetStaticMeshManager()->SetMaterial(mid, m_meshMaterial);
            m_scene->GetStaticMeshManager()->SetMesh(mid, m_meshMesh);
        }
    }

    std::unique_ptr<edit_Command>
    edit_CommandDeleteEntity::Create(const game_Entity& entity, game_Scene* scene)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandDeleteEntity(entity, scene));
    }


    // ===============================
    // edit_CommandCreateEntity
    // ===============================
    edit_CommandCreateEntity::edit_CommandCreateEntity(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager)
        : m_entityManager(emanager)
        , m_metaManager(metaManager)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreateEntity::Do()
    {
        diag_AssertWithReason(m_createdEntity == game_EntityId_Invalid, "Same entity can't be created twice!");
        m_createdEntity = m_entityManager->CreateEntity();
        game_EntityMetaData meta;
        meta.entity = m_createdEntity;
        std::stringstream ss;
        ss << "Entity [" << m_createdEntity.id << "]";
        strcpy_s(meta.name, ss.str().c_str());
        m_metaManager->CreateMetaData(m_createdEntity, meta);
    }

    void
    edit_CommandCreateEntity::Undo()
    {
        diag_Assert(m_createdEntity != game_EntityId_Invalid);
        m_entityManager->DestroyEntity(m_createdEntity);
        m_createdEntity = game_EntityId_Invalid;
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateEntity::Create(game_EntityManager* emanager, game_EntityMetaDataManager* metaManager)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateEntity(emanager, metaManager));
    }


    // ===============================
    // edit_CommandCreateDirectionalLight
    // ===============================
    edit_CommandCreateDirectionalLight::edit_CommandCreateDirectionalLight(game_EntityManager*         emanager,
                                                                           game_EntityMetaDataManager* metaManager,
                                                                           game_TransformManager*      tmanager,
                                                                           game_LightManager*          lmanager)
        : m_entityManager(emanager)
        , m_metaManager(metaManager)
        , m_transformManager(tmanager)
        , m_lightManager(lmanager)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreateDirectionalLight::Do()
    {
        diag_AssertWithReason(m_createdEntity == game_EntityId_Invalid, "Same entity can't be created twice!");

        m_createdEntity = m_entityManager->CreateEntity();
        game_EntityMetaData meta;
        meta.entity = m_createdEntity;
        std::stringstream ss;
        ss << "DirLight [" << m_createdEntity.id << "]";
        strcpy_s(meta.name, ss.str().c_str());
        m_metaManager->CreateMetaData(m_createdEntity, meta);

        m_transformManager->CreateTransform(m_createdEntity);

        game_DirectionalLight dlight;
        dlight.direction = math_Vec3(0, 0, -1);
        dlight.color     = math_Vec3::One();
        dlight.strength  = 1.0f;
        m_lightManager->CreateDirectionalLight(m_createdEntity, dlight);
    }

    void
    edit_CommandCreateDirectionalLight::Undo()
    {
        diag_Assert(m_createdEntity != game_EntityId_Invalid);
        m_entityManager->DestroyEntity(m_createdEntity);
        m_createdEntity = game_EntityId_Invalid;
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreateDirectionalLight::Create(game_EntityManager*         emanager,
                                               game_EntityMetaDataManager* metaManager,
                                               game_TransformManager*      tmanager,
                                               game_LightManager*          lmanager)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreateDirectionalLight(emanager, metaManager, tmanager, lmanager));
    }


    // ===============================
    // edit_CommandCreatePointLight
    // ===============================
    edit_CommandCreatePointLight::edit_CommandCreatePointLight(game_EntityManager*         emanager,
                                                               game_EntityMetaDataManager* metaManager,
                                                               game_TransformManager*      tmanager,
                                                               game_LightManager*          lmanager)
        : m_entityManager(emanager)
        , m_metaManager(metaManager)
        , m_transformManager(tmanager)
        , m_lightManager(lmanager)
        , m_createdEntity(game_EntityId_Invalid)
    {}

    void
    edit_CommandCreatePointLight::Do()
    {
        diag_AssertWithReason(m_createdEntity == game_EntityId_Invalid, "Same entity can't be created twice!");

        m_createdEntity = m_entityManager->CreateEntity();
        game_EntityMetaData meta;
        meta.entity = m_createdEntity;
        std::stringstream ss;
        ss << "PointLight [" << m_createdEntity.id << "]";
        strcpy_s(meta.name, ss.str().c_str());
        m_metaManager->CreateMetaData(m_createdEntity, meta);

        m_transformManager->CreateTransform(m_createdEntity);

        game_PointLight plight;
        plight.radius = 10.0f;
        plight.color  = math_Vec3::One();
        m_lightManager->CreatePointLight(m_createdEntity, plight);
    }

    void
    edit_CommandCreatePointLight::Undo()
    {
        diag_Assert(m_createdEntity != game_EntityId_Invalid);
        m_entityManager->DestroyEntity(m_createdEntity);
        m_createdEntity = game_EntityId_Invalid;
    }

    std::unique_ptr<edit_Command>
    edit_CommandCreatePointLight::Create(game_EntityManager*         emanager,
                                         game_EntityMetaDataManager* metaManager,
                                         game_TransformManager*      tmanager,
                                         game_LightManager*          lmanager)
    {
        return std::unique_ptr<edit_Command>(new edit_CommandCreatePointLight(emanager, metaManager, tmanager, lmanager));
    }
} // namespace pge