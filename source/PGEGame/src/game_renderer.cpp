#include "../include/game_renderer.h"
#include "../include/game_world.h"

namespace pge
{
    static const gfx_VertexAttribute SCREEN_MESH_ATTRIBS[]
        = {gfx_VertexAttribute("POSITION", gfx_VertexAttributeType::FLOAT2), gfx_VertexAttribute("TEXTURECOORD", gfx_VertexAttributeType::FLOAT2)};
    static const unsigned SCREEN_MESH_NUM_ATTRIBS = sizeof(SCREEN_MESH_ATTRIBS) / sizeof(SCREEN_MESH_ATTRIBS[0]);

    static const math_Vec2 SCREEN_MESH_VERTICES[] = {math_Vec2(-1, 1),
                                                     math_Vec2(0, 0),
                                                     math_Vec2(-1, -1),
                                                     math_Vec2(0, 1),
                                                     math_Vec2(1, -1),
                                                     math_Vec2(1, 1),
                                                     math_Vec2(1, 1),
                                                     math_Vec2(1, 0)};
    static const unsigned  SCREEN_MESH_INDICES[]  = {0, 1, 2, 2, 3, 0};



    game_Renderer::game_Renderer(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice, res_ResourceManager* resources)
        : m_graphicsAdapter(graphicsAdapter)
        , m_graphicsDevice(graphicsDevice)
        , m_cbTransform(graphicsAdapter, nullptr, sizeof(CBTransform), gfx_BufferUsage::DYNAMIC)
        , m_cbBones(graphicsAdapter, nullptr, sizeof(CBBones), gfx_BufferUsage::DYNAMIC)
        , m_cbLights(graphicsAdapter, nullptr, sizeof(CBLights), gfx_BufferUsage::DYNAMIC)
        , m_cbLightTransforms(graphicsAdapter, nullptr, sizeof(CBLightTransforms), gfx_BufferUsage::DYNAMIC)
        , m_shadowMap(graphicsAdapter, 2048, 2048, true, true, gfx_PixelFormat::R32_FLOAT)
        , m_viewShadows(graphicsAdapter, 1600, 900, true, true, gfx_PixelFormat::R32G32B32A32_FLOAT)
        , m_depthFX(resources->GetEffect("data/effects/depth.effect"))
        , m_shadowFX(resources->GetEffect("data/effects/shadow.effect"))
        , m_multisampleFX(resources->GetEffect("data/effects/multisample.effect"))
        , m_screenMesh(graphicsAdapter,
                       SCREEN_MESH_ATTRIBS,
                       SCREEN_MESH_NUM_ATTRIBS,
                       SCREEN_MESH_VERTICES,
                       sizeof(SCREEN_MESH_VERTICES),
                       SCREEN_MESH_INDICES,
                       sizeof(SCREEN_MESH_INDICES) / sizeof(unsigned))
    {}

    void
    game_Renderer::SetCamera(const math_Mat4x4& cameraView, const math_Mat4x4& cameraProj)
    {
        m_cameraView = cameraView;
        m_cameraProj = cameraProj;
    }

    void
    game_Renderer::UpdateLights(const game_LightManager&     lmanager,
                                const game_TransformManager& tmanager,
                                const game_EntityManager&    emanager,
                                const game_MeshManager&      mmanager,
                                const game_AnimationManager& amanager)
    {
        // Update directional lights
        {
            size_t                       dirCount = 0;
            const game_DirectionalLight* dlights  = lmanager.GetDirectionalLights(&dirCount);
            for (size_t i = 0; i < dirCount; ++i) {
                auto& dlight = m_cbLightsData.dirLights[i];
                if (!emanager.IsEntityAlive(dlights[i].entity)) {
                    continue;
                }
                game_TransformId tid      = tmanager.GetTransformId(dlights[i].entity);
                math_Quat        rotation = tid == game_TransformId_Invalid ? math_Quat() : tmanager.GetLocalRotation(tid);
                dlight.direction          = m_cameraView * math_Rotate(math_Vec4(dlights[i].direction, 0), rotation);
                dlight.color              = math_Vec4(dlights[i].color, dlights[i].strength);

                // TODO: Per-light
                // Shadow map
                if (i == 0) {
                    // static float OrthoWidth  = 50.0f;
                    // static float OrthoHeight = 50.0f;
                    // static float OrthoNear   = 1.0f;
                    // static float OrthoFar    = 50.0f;

                    static float OrthoWidth  = 13.0f;
                    static float OrthoHeight = 17.0f;
                    static float OrthoNear   = 0.0f;
                    static float OrthoFar    = 14.0f;


                    core_Verify(math_Invert(tmanager.GetWorldMatrix(tid), &m_cbLightTransformsData.view));
                    m_cbLightTransformsData.proj = math_OrthographicRH(OrthoWidth, OrthoHeight, OrthoNear, OrthoFar);
                    m_cbLightTransforms.Update(&m_cbLightTransformsData, sizeof(CBLightTransforms));

                    struct {
                        math_Mat4x4 view, proj;
                    } old;
                    old.view     = m_cameraView;
                    old.proj     = m_cameraProj;
                    auto prevRTV = gfx_RenderTarget_GetActiveRTV();

                    SetCamera(m_cbLightTransformsData.view, m_cbLightTransformsData.proj);
                    m_shadowMap.Bind();
                    m_shadowMap.Clear();
                    m_graphicsDevice->SetRasterizerState(gfx_RasterizerState::SOLID_CULL_FRONT);
                    mmanager.DrawMeshes(this, tmanager, amanager, emanager, game_RenderPass::DEPTH);
                    m_graphicsDevice->SetRasterizerState(gfx_RasterizerState::SOLID_CULL_BACK);

                    SetCamera(old.view, old.proj);
                    if (prevRTV == nullptr) {
                        gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
                    } else {
                        prevRTV->Bind();
                    }
                }
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
            const game_PointLight* plights    = lmanager.GetPointLights(&pointCount);
            for (size_t i = 0; i < pointCount; ++i) {
                auto& plight = m_cbLightsData.pointLights[i];
                if (!emanager.IsEntityAlive(plights[i].entity)) {
                    plight.position = math_Vec4::Zero();
                    plight.color    = math_Vec3::Zero();
                    plight.radius   = 0;
                    continue;
                }
                game_TransformId tid = tmanager.GetTransformId(plights[i].entity);
                plight.position = m_cameraView * math_Vec4((tid == game_TransformId_Invalid) ? math_Vec3::Zero() : tmanager.GetWorldPosition(tid), 1);
                plight.color    = plights[i].color;
                plight.radius   = plights[i].radius;
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
        dlight.direction = m_cameraView * math_Vec4(light.direction, 0);
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
    game_Renderer::DrawMesh(const res_Mesh* mesh, const res_Material* material, const math_Mat4x4& modelMatrix, const game_RenderPass& pass)
    {
        core_Assert(mesh != nullptr && material != nullptr);

        m_cbTransformData.viewMatrix  = m_cameraView;
        m_cbTransformData.projMatrix  = m_cameraProj;
        m_cbTransformData.modelMatrix = modelMatrix;
        core_Verify(math_Invert(modelMatrix, &m_cbTransformData.normalMatrix));
        math_Transpose(m_cbTransformData.normalMatrix);
        m_cbTransform.Update(&m_cbTransformData, sizeof(CBTransform));

        m_cbTransform.BindVS(0);
        mesh->Bind();

        switch (pass) {
            case game_RenderPass::DEPTH: {
                m_depthFX->Bind();
                m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
            } break;

            case game_RenderPass::SHADOW: {
                //auto prevRTV = gfx_RenderTarget_GetActiveRTV();

                //m_viewShadows.Bind();
                //m_viewShadows.Clear();

                m_shadowFX->Bind();
                m_cbLightTransforms.BindVS(1);
                m_cbLights.BindPS(1);

                unsigned slot = material->GetEffect()->GetTextureSlot("ShadowMap");
                m_shadowMap.BindTexture(slot);
                m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
                gfx_Texture2D_Unbind(m_graphicsAdapter, slot);

                //if (prevRTV == nullptr) {
                //    gfx_RenderTarget_BindMainRTV(m_graphicsAdapter);
                //} else {
                //    prevRTV->Bind();
                //}

                //DrawRenderToView(&m_viewShadows, m_multisampleFX);
                //gfx_Texture2D_Unbind(m_graphicsAdapter, 0);
            } break;

            case game_RenderPass::LIGHTING: {
                m_shadowFX->Bind();
                m_cbLightTransforms.BindVS(1);
                m_cbLights.BindPS(1);
                material->Bind();
                unsigned slot = material->GetEffect()->GetTextureSlot("ShadowMap");
                m_shadowMap.BindTexture(slot);
                m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
                gfx_Texture2D_Unbind(m_graphicsAdapter, slot);
            } break;

            default: {
                core_CrashAndBurn("Unhandled render pass");
            } break;
        }
    }

    void
    game_Renderer::DrawSkeletalMesh(const res_Mesh*        mesh,
                                    const res_Material*    material,
                                    const math_Mat4x4&     modelMatrix,
                                    const anim_Skeleton&   skeleton,
                                    const game_RenderPass& pass)
    {
        core_Assert(mesh != nullptr && material != nullptr);

        m_cbTransformData.viewMatrix  = m_cameraView;
        m_cbTransformData.projMatrix  = m_cameraProj;
        m_cbTransformData.modelMatrix = modelMatrix;
        core_Verify(math_Invert(modelMatrix, &m_cbTransformData.normalMatrix));
        math_Transpose(m_cbTransformData.normalMatrix);
        m_cbTransform.Update(&m_cbTransformData, sizeof(CBTransform));

        const auto& boneOffsetMatrices = mesh->GetBoneOffsetMatrices();
        unsigned    numBones           = skeleton.GetBoneCount();
        for (unsigned i = 0; i < numBones; ++i) {
            m_cbBonesData.bones[i] = skeleton.GetBone(i).worldTransform * boneOffsetMatrices[i];
        }
        m_cbBones.Update(&m_cbBonesData, sizeof(CBBones));

        m_cbTransform.BindVS(0);
        m_cbBones.BindVS(1);

        m_cbLights.BindPS(1);
        mesh->Bind();
        material->Bind();

        m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh->GetNumTriangles() * 3);
    }

    void
    game_Renderer::DrawRenderToView(const gfx_RenderTarget* rt, const res_Effect* effect)
    {
        m_screenMesh.Bind();
        effect->Bind();
        rt->BindTexture(0);
        m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, 6);
    }
} // namespace pge