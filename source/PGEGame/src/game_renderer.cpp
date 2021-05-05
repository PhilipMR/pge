#include "../include/game_renderer.h"

namespace pge
{
    game_Renderer::game_Renderer(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice)
        : m_graphicsDevice(graphicsDevice)
        , m_cbTransform(graphicsAdapter, nullptr, sizeof(CBTransform), gfx_BufferUsage::DYNAMIC)
        , m_cbLights(graphicsAdapter, nullptr, sizeof(CBLights), gfx_BufferUsage::DYNAMIC)
    {}

    void
    game_Renderer::SetCamera(const game_Camera* camera)
    {
        m_camera = camera;
    }

    void
    game_Renderer::UpdateLights(const game_LightManager& lightManager, const game_TransformManager& tmanager)
    {
        // Update directional lights
        {
            size_t                       dirCount = 0;
            const game_DirectionalLight* dlights  = lightManager.GetDirectionalLights(&dirCount);
            for (size_t i = 0; i < dirCount; ++i) {
                auto&            dlight   = m_cbLightsData.dirLights[i];
                game_TransformId tid      = tmanager.GetTransformId(dlights[i].entity);
                math_Quat        rotation = tid == game_TransformId_Invalid ? math_Quat() : tmanager.GetLocalRotation(tid);
                dlight.direction          = m_camera->GetViewMatrix() * math_Rotate(math_Vec4(dlights[i].direction, 0), rotation);
                dlight.color              = math_Vec4(dlights[i].color, dlights[i].strength);
            }
            for (size_t i = dirCount; i < MAX_DIRLIGHTS; ++i) {
                auto& dlight     = m_cbLightsData.dirLights[i];
                dlight.direction = math_Vec4::Zero();
                dlight.color     = math_Vec4::Zero();
            }
        }


        // Update point lights
        {
            size_t                 pointCount = 0;
            const game_PointLight* plights    = lightManager.GetPointLights(&pointCount);
            for (size_t i = 0; i < pointCount; ++i) {
                auto&            plight = m_cbLightsData.pointLights[i];
                game_TransformId tid    = tmanager.GetTransformId(plights[i].entity);
                plight.position         = m_camera->GetViewMatrix()
                                  * math_Vec4((tid == game_TransformId_Invalid) ? math_Vec3::Zero() : tmanager.GetWorldPosition(tid), 1);
                plight.color  = plights[i].color;
                plight.radius = plights[i].radius;
            }
            for (size_t i = pointCount; i < MAX_POINTLIGHTS; ++i) {
                auto& plight    = m_cbLightsData.pointLights[i];
                plight.position = math_Vec4::Zero();
                plight.color    = math_Vec3::Zero();
                plight.radius   = 0;
            }
        }

        m_cbLights.Update(&m_cbLightsData, sizeof(CBLights));
    }

    void
    game_Renderer::SetDirectionalLight(size_t slot, const game_DirectionalLight& light)
    {
        core_Assert(slot < MAX_DIRLIGHTS);
        auto& dlight     = m_cbLightsData.dirLights[slot];
        dlight.direction = m_camera->GetViewMatrix() * math_Vec4(light.direction, 0);
        dlight.color     = math_Vec4(light.color, light.strength);
        m_cbLights.Update(&m_cbLightsData, sizeof(CBLights));
    }

    void
    game_Renderer::SetPointLight(size_t slot, const game_PointLight& light, const math_Vec3& position)
    {
        core_Assert(slot < MAX_POINTLIGHTS);
        auto& plight    = m_cbLightsData.pointLights[slot];
        plight.position = math_Vec4(position, 1);
        plight.color    = plight.color;
        plight.radius   = plight.radius;
        m_cbLights.Update(&m_cbLightsData, sizeof(CBLights));
    }

    void
    game_Renderer::DrawMesh(const res_Mesh* mesh, const res_Material* material, const math_Mat4x4& modelMatrix)
    {
        core_Assert(mesh != nullptr && material != nullptr);

        m_cbTransformData.viewMatrix  = m_camera->GetViewMatrix();
        m_cbTransformData.projMatrix  = m_camera->GetProjectionMatrix();
        m_cbTransformData.modelMatrix = modelMatrix;
        m_cbTransform.Update(&m_cbTransformData, sizeof(CBTransform));

        m_cbTransform.BindVS(0);
        m_cbLights.BindPS(1);
        mesh->Bind();
        material->Bind();

        m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
    }
} // namespace pge