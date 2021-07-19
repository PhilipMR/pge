#include "../include/game_world.h"
#include <gfx_debug_draw.h>

namespace pge
{
    game_World::game_World(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_transformManager(100)
        , m_meshManager(100, resources)
        , m_animationManager(100)
        , m_lightManager(&m_transformManager, 100)
        , m_scriptManager(100)
        , m_behaviourManager()
        , m_cameraManager(&m_transformManager)
        , m_renderer(graphicsAdapter, graphicsDevice, resources)
    {}

    void
    game_World::GarbageCollect()
    {
        m_meshManager.GarbageCollect(m_entityManager);
        m_animationManager.GarbageCollect(m_entityManager);
        m_transformManager.GarbageCollect(m_entityManager);
        m_lightManager.GarbageCollect(m_entityManager);
        m_scriptManager.GarbageCollect(m_entityManager);
        m_cameraManager.GarbageCollect(m_entityManager);
    }

    void
    game_World::Update()
    {
        m_behaviourManager.Update(1.0f / 60.0f);
        m_animationManager.Update(1.0f / 60.0f);
        m_scriptManager.UpdateScripts();
    }


    void
    game_World::Draw(const math_Mat4x4& view, const math_Mat4x4& proj, const game_RenderPass& pass, bool withDebug)
    {
        m_scriptManager.UpdateScripts();

        m_renderer.SetCamera(view, proj);
        m_renderer.UpdateLights(m_lightManager, m_transformManager, m_entityManager);
        m_meshManager.DrawMeshes(&m_renderer, m_transformManager, m_animationManager, m_entityManager, pass);

        if (withDebug) {
            gfx_DebugDraw_SetView(view);
            gfx_DebugDraw_SetProjection(proj);
            gfx_DebugDraw_Render();
        }
    }

    void
    game_World::Draw()
    {
        const game_Entity& camera     = m_cameraManager.GetActiveCamera();
        const math_Mat4x4& cameraView = m_cameraManager.GetViewMatrix(camera);
        const math_Mat4x4& cameraProj = m_cameraManager.GetProjectionMatrix(camera);
        Draw(cameraView, cameraProj, game_RenderPass::LIGHTING, true);
    }


    game_Entity
    game_World::FindEntityAtCursor(const math_Vec2& cursor, const math_Vec2& viewSize, const math_Mat4x4& viewMat, const math_Mat4x4& projMat) const
    {
        math_Vec2 billboardSize(2, 2);

        // Static Mesh select
        const math_Mat4x4 viewProj = projMat * viewMat;
        const math_Ray    ray      = math_Raycast_RayFromPixel(cursor, viewSize, viewProj);
        float             meshSelectDistance;
        game_Entity       meshSelectEntity = m_meshManager.RaycastSelect(m_transformManager, ray, viewProj, &meshSelectDistance);

        // Point Light select
        math_Vec2 cursorNorm(cursor.x / viewSize.x, cursor.y / viewSize.y);

        float       lightSelectDistance;
        game_Entity lightSelectEntity = m_lightManager.FindLightAtCursor(cursorNorm, billboardSize, viewMat, projMat, &lightSelectDistance);

        float       cameraSelectDistance;
        game_Entity cameraSelectEntity = m_cameraManager.FindCameraAtCursor(cursorNorm, billboardSize, viewMat, projMat, &cameraSelectDistance);

        struct Intersection {
            game_Entity entity;
            float       distance;
        };
        Intersection meshIntersect{meshSelectEntity, meshSelectDistance};
        Intersection lightIntersect{lightSelectEntity, lightSelectDistance};
        Intersection cameraIntersect{cameraSelectEntity, cameraSelectDistance};

        auto ClosestIntersection2 = [](const Intersection& a, const Intersection& b) {
            return a.distance <= b.distance ? a.entity : b.entity;
        };
        auto ClosestIntersection3 = [](const Intersection& a, const Intersection& b, const Intersection& c) {
            return a.distance <= b.distance ? (a.distance <= c.distance ? a.entity : c.entity) : (b.distance <= c.distance ? b.entity : c.entity);
        };

        // Choose the closest one
        game_Entity whichOne[] = {game_EntityId_Invalid,
                                  meshSelectEntity,
                                  lightSelectEntity,
                                  ClosestIntersection2(lightIntersect, meshIntersect),
                                  cameraSelectEntity,
                                  ClosestIntersection2(cameraIntersect, meshIntersect),
                                  ClosestIntersection2(cameraIntersect, lightIntersect),
                                  ClosestIntersection3(cameraIntersect, lightIntersect, meshIntersect)};

        unsigned selectMeshBit   = unsigned(meshSelectEntity != game_EntityId_Invalid);
        unsigned selectLightBit  = unsigned(lightSelectEntity != game_EntityId_Invalid) << 1;
        unsigned selectCameraBit = unsigned(cameraSelectEntity != game_EntityId_Invalid) << 2;
        return whichOne[selectMeshBit | selectLightBit | selectCameraBit];
    }

    game_EntityManager*
    game_World::GetEntityManager()
    {
        return &m_entityManager;
    }

    game_TransformManager*
    game_World::GetTransformManager()
    {
        return &m_transformManager;
    }

    game_MeshManager*
    game_World::GetMeshManager()
    {
        return &m_meshManager;
    }

    game_AnimationManager*
    game_World::GetAnimationManager()
    {
        return &m_animationManager;
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

    game_BehaviourManager*
    game_World::GetBehaviourManager()
    {
        return &m_behaviourManager;
    }

    game_CameraManager*
    game_World::GetCameraManager()
    {
        return &m_cameraManager;
    }

    game_Renderer*
    game_World::GetRenderer()
    {
        return &m_renderer;
    }


    const game_EntityManager*
    game_World::GetEntityManager() const
    {
        return &m_entityManager;
    }

    const game_TransformManager*
    game_World::GetTransformManager() const
    {
        return &m_transformManager;
    }

    const game_MeshManager*
    game_World::GetMeshManager() const
    {
        return &m_meshManager;
    }

    const game_AnimationManager*
    game_World::GetAnimationManager() const
    {
        return &m_animationManager;
    }

    const game_LightManager*
    game_World::GetLightManager() const
    {
        return &m_lightManager;
    }

    const game_ScriptManager*
    game_World::GetScriptManager() const
    {
        return &m_scriptManager;
    }

    const game_BehaviourManager*
    game_World::GetBehaviourManager() const
    {
        return &m_behaviourManager;
    }

    const game_CameraManager*
    game_World::GetCameraManager() const
    {
        return &m_cameraManager;
    }


    constexpr unsigned SERIALIZE_VERSION             = 1;
    constexpr size_t   SERIALIZED_ENTITY_BUFFER_SIZE = 512;

    constexpr size_t SERIALIZE_TYPE_COMPLETE  = 0;
    constexpr size_t SERIALIZE_TYPE_ENTITY    = 1;
    constexpr size_t SERIALIZE_TYPE_TRANSFORM = 2;
    constexpr size_t SERIALIZE_TYPE_MESH      = 3;
    constexpr size_t SERIALIZE_TYPE_LIGHT     = 4;
    constexpr size_t SERIALIZE_TYPE_SCRIPT    = 5;
    constexpr size_t SERIALIZE_TYPE_CAMERA    = 6;

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

        GarbageCollect();

        if (m_entityManager.IsEntityAlive(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_ENTITY, sizeof(SERIALIZE_TYPE_ENTITY));
            m_entityManager.SerializeEntity(sentitystream, entity);
        }
        if (m_transformManager.HasTransform(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_TRANSFORM, sizeof(SERIALIZE_TYPE_TRANSFORM));
            m_transformManager.SerializeEntity(sentitystream, entity);
        }
        if (m_meshManager.HasMesh(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_MESH, sizeof(SERIALIZE_TYPE_MESH));
            m_meshManager.SerializeEntity(sentitystream, entity);
        }
        if (m_lightManager.HasPointLight(entity) || m_lightManager.HasDirectionalLight(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_LIGHT, sizeof(SERIALIZE_TYPE_LIGHT));
            m_lightManager.SerializeEntity(sentitystream, entity);
        }
        if (m_scriptManager.HasScript(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_SCRIPT, sizeof(SERIALIZE_TYPE_SCRIPT));
            m_scriptManager.SerializeEntity(sentitystream, entity);
        }
        if (m_cameraManager.HasCamera(entity)) {
            sentitystream.write((const char*)&SERIALIZE_TYPE_CAMERA, sizeof(SERIALIZE_TYPE_CAMERA));
            m_cameraManager.SerializeEntity(sentitystream, entity);
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

        bool readingStream = true;
        while (readingStream) {
            size_t compType;
            sentitystream.read((char*)&compType, sizeof(compType));
            switch (compType) {
                case SERIALIZE_TYPE_COMPLETE: {
                    readingStream = false;
                } break;
                case SERIALIZE_TYPE_ENTITY: {
                    m_entityManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_TRANSFORM: {
                    m_transformManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_MESH: {
                    m_meshManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_LIGHT: {
                    m_lightManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_SCRIPT: {
                    m_scriptManager.InsertSerializedEntity(sentitystream, entity);
                } break;
                case SERIALIZE_TYPE_CAMERA: {
                    m_cameraManager.InsertSerializedEntity(sentitystream, entity);
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
    operator<<(std::ostream& os, const game_World& world)
    {
        os << SERIALIZE_VERSION;
        os << world.m_entityManager;
        os << world.m_transformManager;
        os << world.m_meshManager;
        os << world.m_lightManager;
        os << world.m_cameraManager;
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_World& world)
    {
        unsigned version = 0;
        is >> version;
        is >> world.m_entityManager;
        is >> world.m_transformManager;
        is >> world.m_meshManager;
        is >> world.m_lightManager;
        is >> world.m_cameraManager;
        return is;
    }
} // namespace pge