#ifndef PGE_GAME_GAME_STATIC_MESH_H
#define PGE_GAME_GAME_STATIC_MESH_H

#include "game_entity.h"
#include "game_transform.h"
#include "game_renderer.h"
#include "game_animation.h"

#include <math_raycasting.h>
#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <res_mesh.h>
#include <res_material.h>
#include <res_resource_manager.h>
#include <unordered_map>

namespace pge
{
    using game_MeshId                         = unsigned;
    static const unsigned game_MeshId_Invalid = -1;
    class game_MeshManager {
        struct MeshEntity {
            game_Entity         entity;
            const res_Mesh*     mesh;
            const res_Material* material;
        };
        std::vector<MeshEntity>                      m_meshes;
        std::unordered_map<game_Entity, game_MeshId> m_entityMap;
        res_ResourceManager*                         m_resources;

    public:
        game_MeshManager(size_t capacity, res_ResourceManager* resources);

        game_MeshId CreateMesh(const game_Entity& entity);
        game_MeshId CreateMesh(const game_Entity& entity, const res_Mesh* mesh, const res_Material* material);
        void        CreateMeshes(const game_Entity* entities, size_t numEntities, game_MeshId* destBuf);
        void        DestroyMesh(const game_MeshId& id);
        void        GarbageCollect(const game_EntityManager& entityManager);

        bool        HasMesh(const game_Entity& entity) const;
        game_MeshId GetMeshId(const game_Entity& entity) const;

        void SetMesh(const game_MeshId& id, const res_Mesh* mesh);
        void SetMaterial(const game_MeshId& id, const res_Material* material);

        game_Entity         GetEntity(const game_MeshId& id) const;
        const res_Mesh*     GetMesh(const game_MeshId& id) const;
        const res_Material* GetMaterial(const game_MeshId& id) const;

        void DrawMeshes(game_Renderer* renderer, const game_TransformManager& tm, const game_AnimationManager& am, const game_EntityManager& em, const game_RenderPass& pass);
        game_Entity RaycastSelect(const game_TransformManager& tm, const math_Ray& ray, const math_Mat4x4& viewProj, float* distanceOut) const;

        void SerializeEntity(std::ostream& os, const game_Entity& entity) const;
        void InsertSerializedEntity(std::istream& is, const game_Entity& entity);

        friend std::ostream& operator<<(std::ostream& os, const game_MeshManager& sm);
        friend std::istream& operator>>(std::istream& is, game_MeshManager& sm);
    };
} // namespace pge

#endif