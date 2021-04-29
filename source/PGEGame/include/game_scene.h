#ifndef PGE_GAME_GAME_SCENE_H
#define PGE_GAME_GAME_SCENE_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_static_mesh.h"
#include "game_camera.h"
#include "game_light.h"
#include "game_script.h"
#include "game_renderer.h"

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
        game_ScriptManager         m_scriptManager;
        game_Camera                m_camera;
        game_Renderer              m_renderer;

    public:
        game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);
        void Update();
        void Draw();

        game_EntityManager*         GetEntityManager();
        game_EntityMetaDataManager* GetEntityMetaDataManager();
        game_TransformManager*      GetTransformManager();
        game_StaticMeshManager*     GetStaticMeshManager();
        game_LightManager*          GetLightManager();
        game_ScriptManager*         GetScriptManager();

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

        friend std::ostream& operator<<(std::ostream& os, const game_Scene& scene);
        friend std::istream& operator>>(std::istream& is, game_Scene& scene);
    };
} // namespace pge


#endif