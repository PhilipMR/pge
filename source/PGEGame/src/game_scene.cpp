#include "../include/game_scene.h"

namespace pge
{
    game_Scene::game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_transformManager(100)
        , m_staticMeshManager(100, graphicsAdapter, graphicsDevice, resources)
        , m_lightManager(100)
        , m_scriptManager(100)
        , m_renderer(graphicsAdapter, graphicsDevice)
    {}

    void
    game_Scene::Update()
    {
        game_Camera_UpdateFPS(&m_camera, .1f);
        m_entityMetaManager.GarbageCollect(m_entityManager);
        m_staticMeshManager.GarbageCollect(m_entityManager);
        m_transformManager.GarbageCollect(m_entityManager);
        m_lightManager.GarbageCollect(m_entityManager);
        m_scriptManager.GarbageCollect(m_entityManager);
    }

    void
    game_Scene::Draw()
    {
        m_scriptManager.UpdateScripts();

        m_renderer.SetCamera(&m_camera);
        m_renderer.UpdateLights(m_lightManager, m_transformManager);
        m_staticMeshManager.DrawStaticMeshes(&m_renderer, m_transformManager);
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

    game_LightManager*
    game_Scene::GetLightManager()
    {
        return &m_lightManager;
    }

    game_ScriptManager*
    game_Scene::GetScriptManager()
    {
        return &m_scriptManager;
    }

    constexpr unsigned SERIALIZE_VERSION = 1;

    std::ostream&
    operator<<(std::ostream& os, const game_Scene& scene)
    {
        os << SERIALIZE_VERSION;
        os << scene.m_entityManager;
        os << scene.m_entityMetaManager;
        os << scene.m_transformManager;
        os << scene.m_staticMeshManager;
        os << scene.m_lightManager;
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_Scene& scene)
    {
        int version = 0;
        is >> version;
        is >> scene.m_entityManager;
        is >> scene.m_entityMetaManager;
        is >> scene.m_transformManager;
        is >> scene.m_staticMeshManager;
        is >> scene.m_lightManager;
        return is;
    }
} // namespace pge