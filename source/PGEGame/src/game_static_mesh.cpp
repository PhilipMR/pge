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

    game_StaticMeshId
    game_StaticMeshManager::CreateStaticMesh(const game_Entity& entity)
    {
        diag_Assert(!HasStaticMesh(entity));
        diag_Assert(m_meshes.size() < m_meshes.capacity());

        StaticMeshEntity meshEntity;
        meshEntity.entity   = entity;
        meshEntity.mesh     = nullptr;
        meshEntity.material = nullptr;
        m_meshes.push_back(meshEntity);

        game_StaticMeshId meshId = m_meshes.size() - 1;
        m_entityMap.insert(std::make_pair(entity, meshId));
        return meshId;
    }

    void
    game_StaticMeshManager::CreateStaticMeshes(const game_Entity* entities, size_t numEntities, game_StaticMeshId* destBuf)
    {
        for (size_t i = 0; i < numEntities; ++i) {
            destBuf[i] = CreateStaticMesh(entities[i]);
        }
    }

    void
    game_StaticMeshManager::DestroyStaticMesh(const game_StaticMeshId& id)
    {
        diag_Assert(id < m_entityMap.size());

        const game_StaticMeshId& delId  = id;
        const game_Entity        delEnt = m_meshes[delId].entity;

        const game_StaticMeshId& lastId  = m_entityMap.size() - 1;
        const game_Entity        lastEnt = m_meshes[lastId].entity;

        if (delId != lastId) {
            m_meshes[delId]        = m_meshes[lastId];
            m_meshes[delId].entity = lastEnt;
            m_entityMap[lastEnt]   = delId;
        }
        m_entityMap.erase(delEnt);
        m_meshes.pop_back();
    }

    void
    game_StaticMeshManager::GarbageCollect(const game_EntityManager& entityManager)
    {
        for (size_t aliveStreak = 0; m_meshes.size() > 0 && aliveStreak < 4;) {
            unsigned randIdx = rand() % m_meshes.size();
            if (!entityManager.IsEntityAlive(m_meshes[randIdx].entity)) {
                DestroyStaticMesh(randIdx);
                aliveStreak = 0;
            } else {
                aliveStreak++;
            }
        }
    }

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

    const res_Material*
    game_StaticMeshManager::GetMaterial(const game_StaticMeshId& id) const
    {
        diag_Assert(id < m_meshes.size());
        return m_meshes[id].material;
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

            m_cbTransformsData.modelMatrix = tm.HasTransform(mesh.entity) ? tm.GetWorld(tm.GetTransformId(mesh.entity)) : math_Mat4x4();
            m_cbTransforms.Update(&m_cbTransformsData, sizeof(CBTransforms));
            m_graphicsDevice->DrawIndexed(gfx_PrimitiveType::TRIANGLELIST, 0, mesh.mesh->GetNumTriangles() * 3);
        }
    }

    game_StaticMeshId
    game_StaticMeshManager::RaycastSelect(const game_TransformManager& tm, const math_Ray& ray, const math_Mat4x4& viewProj) const
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
        size_t   pos       = is.tellg();
        bool     iseof     = is.eof();
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
            sm.m_meshes[i].mesh     = sm.m_resources->GetMesh(meshPath);
            sm.m_meshes[i].material = sm.m_resources->GetMaterial(matPath);
        }

        sm.m_entityMap.clear();
        for (size_t i = 0; i < numMeshes; ++i) {
            sm.m_entityMap.insert(std::make_pair<>(sm.m_meshes[i].entity, game_StaticMeshId(i)));
        }
        return is;
    }
} // namespace pge