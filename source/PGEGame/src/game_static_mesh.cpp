#include "../include/game_static_mesh.h"
#include <diag_assert.h>
#include <math_mat4x4.h>

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
            destBuf[i]               = meshId;

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

    game_Entity
    game_StaticMeshManager::GetEntity(const game_StaticMeshId& id) const
    {
        diag_Assert(id < m_meshes.size());
        return m_meshes[id].entity;
    }

    const res_Mesh* game_StaticMeshManager::GetMesh(const game_StaticMeshId& id) const
    {
        diag_Assert(id < m_meshes.size());
        return m_meshes[id].mesh;
    }

    void
    game_StaticMeshManager::DrawStaticMeshes(const game_TransformManager& tm, const math_Mat4x4& view, const math_Mat4x4& proj)
    {
        m_cbTransformsData.viewMatrix = view;
        m_cbTransformsData.projMatrix = proj;

        m_cbTransforms.BindVS(0);
        for (const auto& mesh : m_meshes) {
            mesh.mesh->Bind();
            mesh.material->Bind();

            m_cbTransformsData.modelMatrix = tm.GetWorld(tm.GetTransformId(mesh.entity));
            m_cbTransforms.Update(&m_cbTransformsData, sizeof(CBTransforms));
            m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh.mesh->GetNumTriangles() * 3);
        }
    }

    game_StaticMeshId
    game_StaticMeshManager::GetRaycastStaticMesh(const game_TransformManager& tm, const math_Ray& ray, const math_Mat4x4& viewProj) const
    {
        float             closestDistance = std::numeric_limits<float>::max();
        game_StaticMeshId closestMesh     = game_StaticMeshId_Invalid;
        for (const auto& mesh : m_meshes) {
            auto aabb      = mesh.mesh->GetAABB();
            aabb           = math_TransformAABB(aabb, tm.GetWorld(tm.GetTransformId(mesh.entity)));
            float distance = 0;
            if (math_Raycast_IntersectsAABB(ray, aabb, &distance)) {
                if (distance < closestDistance) {
                    closestMesh     = m_entityMap.find(mesh.entity)->second;
                    closestDistance = distance;
                }
            }
        }
        return closestMesh;
    }
} // namespace pge