#ifndef PGE_GAME_GAME_WORLD_H
#define PGE_GAME_GAME_WORLD_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_static_mesh.h"
#include "game_camera.h"
#include "game_light.h"
#include "game_script.h"
#include "game_behaviour.h"
#include "game_renderer.h"

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;
    using game_SerializedEntity = std::unique_ptr<char[]>;
    class game_World {
        game_EntityManager         m_entityManager;
        game_EntityMetaDataManager m_entityMetaManager;
        game_TransformManager      m_transformManager;
        game_StaticMeshManager     m_staticMeshManager;
        game_LightManager          m_lightManager;
        game_ScriptManager         m_scriptManager;
        game_BehaviourManager      m_behaviourManager;
        game_Camera                m_camera;
        game_Renderer              m_renderer;

    public:
        game_World(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);
        void GarbageCollect();
        void Update();
        void Draw();

        game_EntityManager*         GetEntityManager();
        game_EntityMetaDataManager* GetEntityMetaDataManager();
        game_TransformManager*      GetTransformManager();
        game_StaticMeshManager*     GetStaticMeshManager();
        game_LightManager*          GetLightManager();
        game_ScriptManager*         GetScriptManager();
        game_BehaviourManager*             GetBehaviourManager();

        const game_EntityManager*         GetEntityManager() const;
        const game_EntityMetaDataManager* GetEntityMetaDataManager() const;
        const game_TransformManager*      GetTransformManager() const;
        const game_StaticMeshManager*     GetStaticMeshManager() const;
        const game_LightManager*          GetLightManager() const;
        const game_ScriptManager*         GetScriptManager() const;
        const game_BehaviourManager*             GetBehaviourManager() const;


        inline game_Camera*
        GetCamera()
        {
            return &m_camera;
        }

        inline const game_Camera*
        GetCamera() const
        {
            return &m_camera;
        }

        game_SerializedEntity SerializeEntity(const game_Entity& entity);
        void                  InsertSerializedEntity(const game_SerializedEntity& sentity, const game_Entity& entity);
        game_Entity           InsertSerializedEntity(const game_SerializedEntity& entity);
        friend std::ostream&  operator<<(std::ostream& os, const game_World& scene);
        friend std::istream&  operator>>(std::istream& is, game_World& scene);
    };
} // namespace pge


#endif