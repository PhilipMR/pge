#include "../include/game_scene.h"

namespace pge
{
    game_Scene::game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_transformManager(1000)
        , m_staticMeshManager(1000, graphicsAdapter, graphicsDevice, resources)
    {}

    void
    game_Scene::Update()
    {
        m_camera.UpdateFPS(.1f);
        m_entityMetaManager.GarbageCollect(m_entityManager);
        m_staticMeshManager.GarbageCollect(m_entityManager);
        m_transformManager.GarbageCollect(m_entityManager);
    }

    void
    game_Scene::Draw()
    {
        m_staticMeshManager.DrawStaticMeshes(m_transformManager, m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix());
    }

    game_EntityManager*
    game_Scene::GetEntityManager()
    {
        return &m_entityManager;
    }

    game_EntityMetaDataManager*
    game_Scene::GetEntityMetaDataManager()
    {
        return &m_entityMetaManager;
    }

    game_TransformManager*
    game_Scene::GetTransformManager()
    {
        return &m_transformManager;
    }

    game_StaticMeshManager*
    game_Scene::GetStaticMeshManager()
    {
        return &m_staticMeshManager;
    }

    const game_EntityManager*
    game_Scene::GetEntityManager() const
    {
        return &m_entityManager;
    }

    const game_EntityMetaDataManager*
    game_Scene::GetEntityMetaDataManager() const
    {
        return &m_entityMetaManager;
    }

    const game_TransformManager*
    game_Scene::GetTransformManager() const
    {
        return &m_transformManager;
    }

    const game_StaticMeshManager*
    game_Scene::GetStaticMeshManager() const
    {
        return &m_staticMeshManager;
    }

    std::ostream&
    operator<<(std::ostream& os, const game_Scene& scene)
    {
        os << scene.m_entityManager;
        os << scene.m_entityMetaManager;
        os << scene.m_transformManager;
        os << scene.m_staticMeshManager;
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_Scene& scene)
    {
        is >> scene.m_entityManager;
        is >> scene.m_entityMetaManager;
        is >> scene.m_transformManager;
        is >> scene.m_staticMeshManager;
        return is;
    }
} // namespace pge