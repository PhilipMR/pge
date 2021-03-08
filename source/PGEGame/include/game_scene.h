#ifndef PGE_GAME_GAME_SCENE_H
#define PGE_GAME_GAME_SCENE_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_static_mesh.h"
#include "game_camera.h"
#include "game_light.h"

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;
    class game_Scene {
        game_EntityManager         m_entityManager;
        game_EntityMetaDataManager m_entityMetaManager;
        game_TransformManager      m_transformManager;
        game_StaticMeshManager     m_staticMeshManager;
        game_LightManager          m_lightManager;
        game_FPSCamera             m_camera;

    public:
        game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);
        void Update();
        void Draw();

        game_EntityManager*         GetEntityManager();
        game_EntityMetaDataManager* GetEntityMetaDataManager();
        game_TransformManager*      GetTransformManager();
        game_StaticMeshManager*     GetStaticMeshManager();
        game_LightManager*          GetLightManager();

        const game_EntityManager*         GetEntityManager() const;
        const game_EntityMetaDataManager* GetEntityMetaDataManager() const;
        const game_TransformManager*      GetTransformManager() const;
        const game_StaticMeshManager*     GetStaticMeshManager() const;
        const game_LightManager*          GetLightManager() const;

        inline game_FPSCamera*
        GetCamera()
        {
            return &m_camera;
        }

        inline const game_FPSCamera*
        GetCamera() const
        {
            return &m_camera;
        }

        friend std::ostream& operator<<(std::ostream& os, const game_Scene& scene);
        friend std::istream& operator>>(std::istream& is, game_Scene& scene);
    };
} // namespace pge


#endif