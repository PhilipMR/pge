#ifndef PGE_GAME_GAME_SCENE_H
#define PGE_GAME_GAME_SCENE_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_static_mesh.h"
#include "game_camera.h"

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;
    class game_Scene {
        game_EntityManager         m_entityManager;
        game_EntityMetaDataManager m_entityMetaManager;
        game_TransformManager      m_transformManager;
        game_StaticMeshManager     m_staticMeshManager;
        game_FPSCamera             m_camera;

    public:
        game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice);
        void Update();
        void Draw();

        game_EntityManager*         GetEntityManager();
        game_EntityMetaDataManager* GetEntityMetaDataManager();
        game_TransformManager*      GetTransformManager();
        game_StaticMeshManager*     GetStaticMeshManager();

        const game_EntityManager*         GetEntityManager() const;
        const game_EntityMetaDataManager* GetEntityMetaDataManager() const;
        const game_TransformManager*      GetTransformManager() const;
        const game_StaticMeshManager*     GetStaticMeshManager() const;

        inline game_FPSCamera*
        GetCamera()
        {
            return &m_camera;
        }
    };
} // namespace pge


#endif