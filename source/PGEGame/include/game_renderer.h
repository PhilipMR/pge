#ifndef PGE_GAME_GAME_RENDERER_H
#define PGE_GAME_GAME_RENDERER_H

#include <math_mat4x4.h>
#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <gfx_render_target.h>
#include <anim_skeleton.h>
#include <res_resource_manager.h>

#include "game_light.h"
#include "game_camera.h"

namespace pge
{
    enum class game_RenderPass
    {
        DEPTH,
        SHADOW,
        LIGHTING
    };

    class game_TransformManager;
    class game_EntityManager;
    class game_MeshManager;
    class game_AnimationManager;

    class game_Renderer {
        gfx_GraphicsAdapter* m_graphicsAdapter;
        gfx_GraphicsDevice*  m_graphicsDevice;

        math_Mat4x4 m_cameraView;
        math_Mat4x4 m_cameraProj;

        struct CBTransform {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
            math_Mat4x4 normalMatrix;
        } m_cbTransformData;
        gfx_ConstantBuffer m_cbTransform;

        static const unsigned MAX_BONES = 100;
        struct CBBones {
            math_Mat4x4 bones[MAX_BONES];
        } m_cbBonesData;
        gfx_ConstantBuffer m_cbBones;

        static const unsigned MAX_DIRLIGHTS   = 10;
        static const unsigned MAX_POINTLIGHTS = 10;
        struct CBLights {
            struct {
                math_Vec4 direction;
                math_Vec4 color; // alpha = strength
            } dirLights[MAX_DIRLIGHTS];
            struct {
                math_Vec4 position;
                math_Vec3 color;
                float     radius;
            } pointLights[MAX_POINTLIGHTS];
        } m_cbLightsData;
        gfx_ConstantBuffer m_cbLights;

        struct CBLightTransforms {
            math_Mat4x4 view;
            math_Mat4x4 proj;
        } m_cbLightTransformsData;
        gfx_ConstantBuffer m_cbLightTransforms;


        gfx_RenderTarget m_shadowMap;
        gfx_RenderTarget m_viewShadows;

        const res_Effect* m_depthFX;
        const res_Effect* m_shadowFX;
        const res_Effect* m_multisampleFX;
        const res_Mesh    m_screenMesh;

    public:
        game_Renderer(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources);

        void SetCamera(const math_Mat4x4& cameraView, const math_Mat4x4& cameraProj);

        void UpdateLights(const game_LightManager&     lmanager,
                          const game_TransformManager& tmanager,
                          const game_EntityManager&    emanager,
                          const game_MeshManager&      mmanager,
                          const game_AnimationManager& amanager);
        void SetDirectionalLight(size_t slot, const game_DirectionalLight& light);
        void SetPointLight(size_t slot, const game_PointLight& light, const math_Vec3& position);
        
        void DrawMesh(const res_Mesh* mesh, const res_Material* material, const math_Mat4x4& modelMatrix, const game_RenderPass& pass);
        void DrawSkeletalMesh(const res_Mesh*        mesh,
                              const res_Material*    material,
                              const math_Mat4x4&     modelMatrix,
                              const anim_Skeleton&   skeleton,
                              const game_RenderPass& pass);

        void DrawRenderToView(const gfx_RenderTarget* rt, const res_Effect* effect);
    };
} // namespace pge

#endif