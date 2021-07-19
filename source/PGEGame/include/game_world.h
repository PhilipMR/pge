#ifndef PGE_GAME_GAME_WORLD_H
#define PGE_GAME_GAME_WORLD_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_mesh.h"
#include "game_animation.h"
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
        game_EntityManager    m_entityManager;
        game_TransformManager m_transformManager;
        game_MeshManager      m_meshManager;
        game_AnimationManager m_animationManager;
        game_LightManager     m_lightManager;
        game_ScriptManager    m_scriptManager;
        game_BehaviourManager m_behaviourManager;
        game_CameraManager    m_cameraManager;
        game_Renderer         m_renderer;

    public:
        game_World(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);
        void GarbageCollect();
        void Update();

        void Draw(const math_Mat4x4& view, const math_Mat4x4& proj, const game_RenderPass& pass = game_RenderPass::LIGHTING, bool withDebug = true);
        void Draw();

        game_Entity
        FindEntityAtCursor(const math_Vec2& cursor, const math_Vec2& viewSize, const math_Mat4x4& viewMat, const math_Mat4x4& projMat) const;

        game_EntityManager*    GetEntityManager();
        game_TransformManager* GetTransformManager();
        game_MeshManager*      GetMeshManager();
        game_AnimationManager* GetAnimationManager();
        game_LightManager*     GetLightManager();
        game_ScriptManager*    GetScriptManager();
        game_BehaviourManager* GetBehaviourManager();
        game_CameraManager*    GetCameraManager();
        game_Renderer*         GetRenderer();

        const game_EntityManager*    GetEntityManager() const;
        const game_TransformManager* GetTransformManager() const;
        const game_MeshManager*      GetMeshManager() const;
        const game_AnimationManager* GetAnimationManager() const;
        const game_LightManager*     GetLightManager() const;
        const game_ScriptManager*    GetScriptManager() const;
        const game_BehaviourManager* GetBehaviourManager() const;
        const game_CameraManager*    GetCameraManager() const;


        game_SerializedEntity SerializeEntity(const game_Entity& entity);
        void                  InsertSerializedEntity(const game_SerializedEntity& sentity, const game_Entity& entity);
        game_Entity           InsertSerializedEntity(const game_SerializedEntity& entity);
        friend std::ostream&  operator<<(std::ostream& os, const game_World& world);
        friend std::istream&  operator>>(std::istream& is, game_World& world);
    };
} // namespace pge


#endif