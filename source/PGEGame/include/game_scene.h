#ifndef PGE_GAME_GAME_SCENE_H
#define PGE_GAME_GAME_SCENE_H

#include "game_camera.h"
#include <res_mesh.h>
#include <res_material.h>
#include <gfx_buffer.h>
#include <gfx_graphics_device.h>
#include <vector>

namespace pge
{
    class game_StaticMesh {
        const res_Mesh*     m_mesh;
        const res_Material* m_material;
        game_Transform      m_transform;

    public:
        game_StaticMesh()
            : m_mesh(nullptr)
            , m_material(nullptr)
        {}

        game_StaticMesh(const res_Mesh* mesh, const res_Material* material)
            : m_mesh(mesh)
            , m_material(material)
        {}

        game_StaticMesh(const res_Mesh* mesh, const res_Material* material, const game_Transform& transform)
            : m_mesh(mesh)
            , m_material(material)
            , m_transform(transform)
        {}

        const res_Mesh*
        GetMesh() const
        {
            return m_mesh;
        }

        const res_Material*
        GetMaterial() const
        {
            return m_material;
        }

        game_Transform*
        GetTransform()
        {
            return &m_transform;
        }

        const game_Transform*
        GetTransform() const
        {
            return &m_transform;
        }
    };

    class game_Scene {
        game_Camera                  m_camera;
        std::vector<game_StaticMesh> m_staticMeshes;

        struct CBTransforms {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
        } m_cbTransformsData;
        gfx_GraphicsDevice* m_graphicsDevice;
        gfx_ConstantBuffer  m_cbTransforms;

    public:
        game_Scene(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice)
            : m_graphicsDevice(graphicsDevice)
            , m_cbTransforms(graphicsAdapter, nullptr, sizeof(CBTransforms), gfx_BufferUsage::DYNAMIC)
        {
            m_staticMeshes.reserve(128);
        }

        game_Camera*
        GetCamera()
        {
            return &m_camera;
        }

        game_StaticMesh*
        CreateStaticMesh(const res_Mesh* mesh, const res_Material* material, const game_Transform& transform)
        {
            m_staticMeshes.emplace_back(game_StaticMesh(mesh, material, transform));
            return &m_staticMeshes.back();
        }

        void
        Draw()
        {
            m_cbTransforms.BindVS(0);
            for (const auto& mesh : m_staticMeshes) {
                mesh.GetMesh()->Bind();
                mesh.GetMaterial()->Bind();
                m_cbTransformsData.modelMatrix = mesh.GetTransform()->GetModelMatrix();
                m_cbTransformsData.viewMatrix  = m_camera.GetViewMatrix();
                m_cbTransformsData.projMatrix  = m_camera.GetProjectionMatrix();
                m_cbTransforms.Update(&m_cbTransformsData, sizeof(CBTransforms));
                m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh.GetMesh()->GetNumTriangles() * 3);
            }
        }
    };
} // namespace pge

#endif