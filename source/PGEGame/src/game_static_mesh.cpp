#include "../include/game_static_mesh.h"
#include <diag_assert.h>
#include <math_mat4x4.h>

namespace pge
{
    game_StaticMeshManager::game_StaticMeshManager(size_t               capacity,
                                                   gfx_GraphicsAdapter* graphicsAdapter,
                                                   gfx_GraphicsDevice*  graphicsDevice,
                                                   res_ResourceManager* resources)
        : m_graphicsDevice(graphicsDevice)
        , m_cbTransforms(graphicsAdapter, nullptr, sizeof(CBTransforms), gfx_BufferUsage::DYNAMIC)
        , m_resources(resources)
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

    const res_Mesh*
    game_StaticMeshManager::GetMesh(const game_StaticMeshId& id) const
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

    std::ostream&
    operator<<(std::ostream& os, const game_StaticMeshManager& sm)
    {
        unsigned numMeshes = sm.m_meshes.size();
        os.write((const char*)&numMeshes, sizeof(numMeshes));

        for (const auto& mesh : sm.m_meshes) {
            os.write((const char*)&mesh.entity, sizeof(mesh.entity));

            auto     meshPath    = mesh.mesh->GetPath();
            unsigned meshPathLen = meshPath.size();
            os.write((const char*)&meshPathLen, sizeof(meshPathLen));
            os.write(meshPath.c_str(), meshPathLen);

            auto     matPath    = mesh.material->GetPath();
            unsigned matPathLen = matPath.size();
            os.write((const char*)&matPathLen, sizeof(matPathLen));
            os.write(matPath.c_str(), matPathLen);
        }
        return os;
    }

    std::istream&
    operator>>(std::istream& is, game_StaticMeshManager& sm)
    {
        unsigned numMeshes = 0;
        is.read((char*)&numMeshes, sizeof(numMeshes));

        sm.m_meshes.resize(numMeshes);
        for (unsigned i = 0; i < numMeshes; ++i) {
            game_EntityId entityId;
            is.read((char*)&entityId, sizeof(entityId));

            char     meshPath[128];
            unsigned meshPathLen;
            is.read((char*)&meshPathLen, sizeof(meshPathLen));
            diag_Assert(meshPathLen < 127);
            is.read(meshPath, meshPathLen);
            meshPath[meshPathLen] = 0;

            char     matPath[128];
            unsigned matPathLen;
            is.read((char*)&matPathLen, sizeof(matPathLen));
            diag_Assert(matPathLen < 127);
            is.read(matPath, matPathLen);
            matPath[matPathLen] = 0;

            sm.m_meshes[i].entity   = entityId;
            sm.m_meshes[i].mesh   = sm.m_resources->GetMesh(meshPath);
            sm.m_meshes[i].material = sm.m_resources->GetMaterial(matPath);
        }

        sm.m_entityMap.clear();
        for (size_t i = 0; i < numMeshes; ++i) {
            sm.m_entityMap.insert(std::make_pair<>(sm.m_meshes[i].entity, game_StaticMeshId(i)));
        }
        return is;
    }
} // namespace pge