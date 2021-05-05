#ifndef PGE_GAME_GAME_RENDERER_H
#define PGE_GAME_GAME_RENDERER_H

#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <math_mat4x4.h>
#include <res_mesh.h>
#include <res_material.h>

#include "game_light.h"
#include "game_camera.h"
#include "game_transform.h"

namespace pge
{
    class game_Renderer {
        gfx_GraphicsDevice* m_graphicsDevice;

        const game_Camera* m_camera;

        struct CBTransform {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
        } m_cbTransformData;
        gfx_ConstantBuffer m_cbTransform;

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

    public:
        game_Renderer(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice);

        void SetCamera(const game_Camera* camera);
        void UpdateLights(const game_LightManager& lightManager, const game_TransformManager& tmanager);
        void SetDirectionalLight(size_t slot, const game_DirectionalLight& light);
        void SetPointLight(size_t slot, const game_PointLight& light, const math_Vec3& position);

        void DrawMesh(const res_Mesh* mesh, const res_Material* material, const math_Mat4x4& modelMatrix);
    };
} // namespace pge

#endif