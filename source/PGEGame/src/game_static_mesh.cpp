#include "../include/game_static_mesh.h"
#include <diag_assert.h>
#include <math_mat4x4.h>
#include <gfx_debug_draw.h>

namespace pge
{
    game_StaticMeshManager::game_StaticMeshManager(size_t capacity, gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice)
    : m_graphicsDevice(graphicsDevice)
    , m_cbTransforms(graphicsAdapter, nullptr, sizeof(CBTransforms), gfx_BufferUsage::DYNAMIC)
    {
        m_meshes.reserve(capacity);
    }

    void
    game_StaticMeshManager::CreateStaticMeshes(const game_Entity* entities, size_t numEntities, game_StaticMeshId* destBuf)
    {
        for (size_t i = 0; i < numEntities; ++i) {
            diag_Assert(!HasStaticMesh(entities[i]));
            diag_Assert(m_meshes.size() < m_meshes.capacity());

            StaticMeshEntity meshEntity;
            meshEntity.entity   = entities[i];
            meshEntity.mesh     = nullptr;
            meshEntity.material = nullptr;
            m_meshes.push_back(meshEntity);

            game_StaticMeshId meshId = m_meshes.size() - 1;
            destBuf[i] = meshId;

            m_entityMap.insert(std::make_pair(entities[i], meshId));
        }
    }

    void
    game_StaticMeshManager::DestroyStaticMesh(const game_StaticMeshId& id)
    {}

    bool
    game_StaticMeshManager::HasStaticMesh(const game_Entity& entity) const
    {
        return m_entityMap.find(entity) != m_entityMap.end();
    }

    game_StaticMeshId
    game_StaticMeshManager::GetStaticMeshId(const game_Entity& entity) const
    {
        diag_Assert(HasStaticMesh(entity));
        return m_entityMap.find(entity)->second;
    }

    void
    game_StaticMeshManager::SetMesh(const game_StaticMeshId& id, const res_Mesh* mesh)
    {
        diag_Assert(id < m_meshes.size());
        m_meshes[id].mesh = mesh;
    }

    void
    game_StaticMeshManager::SetMaterial(const game_StaticMeshId& id, const res_Material* material)
    {
        diag_Assert(id < m_meshes.size());
        m_meshes[id].material = material;
    }

    void
    game_StaticMeshManager::DrawStaticMeshes(const game_TransformManager& tm, const math_Mat4x4& view, const math_Mat4x4& proj)
    {
        m_cbTransformsData.viewMatrix  = view;
        m_cbTransformsData.projMatrix  = proj;

        m_cbTransforms.BindVS(0);
        for (const auto& mesh : m_meshes) {
            mesh.mesh->Bind();
            mesh.material->Bind();

            m_cbTransformsData.modelMatrix = tm.GetWorld(tm.GetTransformId(mesh.entity));

            auto aabb = mesh.mesh->GetAABB();
            aabb = math_TransformAABB(aabb, m_cbTransformsData.modelMatrix);
            gfx_DebugDraw_Box(aabb.min, aabb.max);

            m_cbTransforms.Update(&m_cbTransformsData, sizeof(CBTransforms));
            m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh.mesh->GetNumTriangles() * 3);
        }
    }
} // namespace pge