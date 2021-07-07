#include "../include/game_static_mesh.h"
#include <core_assert.h>
#include <math_mat4x4.h>

namespace pge
{
    game_StaticMeshManager::game_StaticMeshManager(size_t capacity, res_ResourceManager* resources)
        : m_resources(resources)
    {
        m_meshes.reserve(capacity);
    }

    game_StaticMeshId
    game_StaticMeshManager::CreateStaticMesh(const game_Entity& entity)
    {
        return CreateStaticMesh(entity, nullptr, nullptr);
    }

    game_StaticMeshId
    game_StaticMeshManager::CreateStaticMesh(const game_Entity& entity, const res_Mesh* mesh, const res_Material* material)
    {
        core_Assert(!HasStaticMesh(entity));
        core_Assert(m_meshes.size() < m_meshes.capacity());

        StaticMeshEntity meshEntity;
        meshEntity.entity   = entity;
        meshEntity.mesh     = mesh;
        meshEntity.material = material;
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
        core_Assert(id < m_entityMap.size());

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
        core_Assert(HasStaticMesh(entity));
        return m_entityMap.find(entity)->second;
    }

    void
    game_StaticMeshManager::SetMesh(const game_StaticMeshId& id, const res_Mesh* mesh)
    {
        core_Assert(id < m_meshes.size());
        m_meshes[id].mesh = mesh;
    }

    void
    game_StaticMeshManager::SetMaterial(const game_StaticMeshId& id, const res_Material* material)
    {
        core_Assert(id < m_meshes.size());
        m_meshes[id].material = material;
    }

    game_Entity
    game_StaticMeshManager::GetEntity(const game_StaticMeshId& id) const
    {
        core_Assert(id < m_meshes.size());
        return m_meshes[id].entity;
    }

    const res_Mesh*
    game_StaticMeshManager::GetMesh(const game_StaticMeshId& id) const
    {
        core_Assert(id < m_meshes.size());
        return m_meshes[id].mesh;
    }

    const res_Material*
    game_StaticMeshManager::GetMaterial(const game_StaticMeshId& id) const
    {
        core_Assert(id < m_meshes.size());
        return m_meshes[id].material;
    }

    void
    game_StaticMeshManager::DrawStaticMeshes(game_Renderer* renderer, const game_TransformManager& tm, const game_AnimationManager& am, const game_EntityManager& em)
    {
        for (const auto& mesh : m_meshes) {
            if (mesh.mesh == nullptr || mesh.material == nullptr || !em.IsEntityAlive(mesh.entity))
                continue;

            math_Mat4x4 modelMatrix = tm.HasTransform(mesh.entity) ? tm.GetWorldMatrix(tm.GetTransformId(mesh.entity)) : math_Mat4x4();
            if (am.HasAnimator(mesh.entity)) {
                renderer->DrawSkeletalMesh(mesh.mesh, mesh.material, modelMatrix, am.GetAnimatedSkeleton(mesh.entity));
            } else {
                renderer->DrawMesh(mesh.mesh, mesh.material, modelMatrix);
            }
        }
    }

    game_Entity
    game_StaticMeshManager::RaycastSelect(const game_TransformManager& tm, const math_Ray& ray, const math_Mat4x4& viewProj, float* distanceOut) const
    {
        float             closestDistance = std::numeric_limits<float>::max();
        game_StaticMeshId closestMesh     = game_StaticMeshId_Invalid;
        for (const auto& mesh : m_meshes) {
            if (!tm.HasTransform(mesh.entity))
                continue;
            auto aabb      = mesh.mesh->GetAABB();
            aabb           = math_TransformAABB(aabb, tm.GetWorldMatrix(tm.GetTransformId(mesh.entity)));
            float distance = 0;
            if (math_Raycast_IntersectsAABB(ray, aabb, &distance)) {
                if (distance < closestDistance) {
                    closestMesh     = m_entityMap.find(mesh.entity)->second;
                    closestDistance = distance;
                }
            }
        }
        if (distanceOut != nullptr)
            *distanceOut = closestDistance;
        return (closestMesh == game_StaticMeshId_Invalid) ? game_EntityId_Invalid : GetEntity(closestMesh);
    }


    constexpr unsigned SERIALIZE_VERSION = 1;

    void
    game_StaticMeshManager::SerializeEntity(std::ostream& os, const game_Entity& entity) const
    {
        game_StaticMeshId mid = m_entityMap.at(entity);

        std::string meshPath    = m_meshes[mid].mesh->GetPath();
        size_t      meshPathLen = meshPath.size();
        os.write((const char*)&meshPathLen, sizeof(meshPathLen));
        os.write((const char*)&meshPath[0], meshPath.size());

        std::string matPath    = m_meshes[mid].material->GetPath();
        size_t      matPathLen = matPath.size();
        os.write((const char*)&matPathLen, sizeof(matPathLen));
        os.write((const char*)&matPath[0], matPath.size());
    }

    void
    game_StaticMeshManager::InsertSerializedEntity(std::istream& is, const game_Entity& entity)
    {
        game_EntityId entityId;
        if (!HasStaticMesh(entity)) {
            CreateStaticMesh(entity);
        }
        game_StaticMeshId mid = m_entityMap.at(entity);

        std::string meshPath;
        size_t      meshPathLen;
        is.read((char*)&meshPathLen, sizeof(meshPathLen));
        meshPath.resize(meshPathLen);
        is.read((char*)&meshPath[0], meshPathLen);

        std::string matPath;
        size_t      matPathLen;
        is.read((char*)&matPathLen, sizeof(matPathLen));
        matPath.resize(matPathLen);
        is.read((char*)&matPath[0], matPathLen);

        SetMesh(mid, m_resources->GetMesh(meshPath.c_str()));
        SetMaterial(mid, m_resources->GetMaterial(matPath.c_str()));
    }


    std::ostream&
    operator<<(std::ostream& os, const game_StaticMeshManager& sm)
    {
        unsigned version = SERIALIZE_VERSION;
        os.write((const char*)&version, sizeof(version));

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
        unsigned version = 0;
        is.read((char*)&version, sizeof(version));

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
            core_Assert(meshPathLen < 127);
            is.read(meshPath, meshPathLen);
            meshPath[meshPathLen] = 0;

            char     matPath[128];
            unsigned matPathLen;
            is.read((char*)&matPathLen, sizeof(matPathLen));
            core_Assert(matPathLen < 127);
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