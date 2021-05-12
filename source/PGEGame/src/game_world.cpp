#include "../include/game_world.h"

namespace pge
{
    game_World::game_World(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_transformManager(100)
        , m_staticMeshManager(100, graphicsAdapter, graphicsDevice, resources)
        , m_lightManager(100)
        , m_scriptManager(100)
        , m_renderer(graphicsAdapter, graphicsDevice)
    {}

    void
    game_World::Update()
    {
        game_Camera_UpdateFPS(&m_camera, .1f);
        m_entityMetaManager.GarbageCollect(m_entityManager);
        m_staticMeshManager.GarbageCollect(m_entityManager);
        m_transformManager.GarbageCollect(m_entityManager);
        m_lightManager.GarbageCollect(m_entityManager);
        m_scriptManager.GarbageCollect(m_entityManager);
    }

    void
    game_World::Draw()
    {
        m_scriptManager.UpdateScripts();

        m_renderer.SetCamera(&m_camera);
        m_renderer.UpdateLights(m_lightManager, m_transformManager);
        m_staticMeshManager.DrawStaticMeshes(&m_renderer, m_transformManager);
    }

    game_EntityManager*
    game_World::GetEntityManager()
    {
        return &m_entityManager;
    }

    game_EntityMetaDataManager*
    game_World::GetEntityMetaDataManager()
    {
        return &m_entityMetaManager;
    }

    game_TransformManager*
    game_World::GetTransformManager()
    {
        return &m_transformManager;
    }

    game_StaticMeshManager*
    game_World::GetStaticMeshManager()
    {
        return &m_staticMeshManager;
    }

    game_LightManager*
    game_World::GetLightManager()
    {
        return &m_lightManager;
    }

    game_ScriptManager*
    game_World::GetScriptManager()
    {
        return &m_scriptManager;
    }

    constexpr unsigned SERIALIZE_VERSION             = 1;
    constexpr size_t   SERIALIZED_ENTITY_BUFFER_SIZE = 512;

    constexpr size_t SERIALIZE_TYPE_COMPLETE  = 0;
    constexpr size_t SERIALIZE_TYPE_META      = 1;
    constexpr size_t SERIALIZE_TYPE_TRANSFORM = 2;
    constexpr size_t SERIALIZE_TYPE_MESH      = 3;
    constexpr size_t SERIALIZE_TYPE_LIGHT     = 4;
    constexpr size_t SERIALIZE_TYPE_SCRIPT    = 5;

    game_SerializedEntity
    game_World::SerializeEntity(const game_Entity& entity)
    {
        struct membuf : std::streambuf {
            membuf(char* p, size_t size)
            {
                setp(p, p + size);
                this->pubseekpos(0, std::ios::out | std::ios::binary);
            }
        };

        const size_t          length = SERIALIZED_ENTITY_BUFFER_SIZE;
        game_SerializedEntity sentity(new char[length]);
        memset(sentity.get(), 0, length);
        membuf       sbuf(sentity.get(), length);
        std::ostream sentitystream(&sbuf);

        if (m_entityMetaManager.HasMetaData(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_META, sizeof(SERIALIZE_TYPE_META));
            m_entityMetaManager.SerializeEntity(sentitystream, entity);
        }
        if (m_transformManager.HasTransform(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_TRANSFORM, sizeof(SERIALIZE_TYPE_TRANSFORM));
            m_transformManager.SerializeEntity(sentitystream, entity);
        }
        if (m_staticMeshManager.HasStaticMesh(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_MESH, sizeof(SERIALIZE_TYPE_MESH));
            m_staticMeshManager.SerializeEntity(sentitystream, entity);
        }
        if (m_lightManager.HasPointLight(entity) || m_lightManager.HasDirectionalLight(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_LIGHT, sizeof(SERIALIZE_TYPE_LIGHT));
            m_lightManager.SerializeEntity(sentitystream, entity);
        }
        if (m_scriptManager.HasScript(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_SCRIPT, sizeof(SERIALIZE_TYPE_SCRIPT));
            m_scriptManager.SerializeEntity(sentitystream, entity);
        }
        sentitystream.write((const char*)&SERIALIZE_TYPE_COMPLETE, sizeof(SERIALIZE_TYPE_COMPLETE));

        return sentity;
    }

    void
    game_World::InsertSerializedEntity(const game_SerializedEntity& sentity, const game_Entity& entity)
    {
        struct membuf : std::streambuf {
            membuf(char* p, size_t size)
            {
                setg(p, p, p + size);
            }
        };
        const size_t length = SERIALIZED_ENTITY_BUFFER_SIZE;
        membuf       sbuf(sentity.get(), length);
        std::istream sentitystream(&sbuf);

        if (!m_entityManager.IsEntityAlive(entity)) {
            m_entityManager.CreateEntity(entity);
        }

        bool readingStream = true;
        while (readingStream) {
            size_t compType;
            sentitystream.read((char*)&compType, sizeof(compType));
            switch (compType) {
                case SERIALIZE_TYPE_COMPLETE: {
                    readingStream = false;
                } break;
                case SERIALIZE_TYPE_META: {
                    m_entityMetaManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_TRANSFORM: {
                    m_transformManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_MESH: {
                    m_staticMeshManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_LIGHT: {
                    m_lightManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_SCRIPT: {
                    m_scriptManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                default: {
                    core_AssertWithReason(false, "Unrecognized component type (data may be corrupted)");
                } break;
            }
        }
    }

    game_Entity
    game_World::InsertSerializedEntity(const game_SerializedEntity& sentity)
    {
        game_Entity entity = m_entityManager.CreateEntity();
        InsertSerializedEntity(sentity, entity);
        return entity;
    }

    std::ostream&
    operator<<(std::ostream& os, const game_World& scene)
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
    operator>>(std::istream& is, game_World& scene)
    {
        unsigned version = 0;
        is >> version;
        is >> scene.m_entityManager;
        is >> scene.m_entityMetaManager;
        is >> scene.m_transformManager;
        is >> scene.m_staticMeshManager;
        is >> scene.m_lightManager;
        return is;
    }
} // namespace pge