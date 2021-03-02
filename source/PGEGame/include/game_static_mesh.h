#ifndef PGE_GAME_GAME_STATIC_MESH_H
#define PGE_GAME_GAME_STATIC_MESH_H

#include "game_entity.h"
#include "game_transform.h"

#include <res_mesh.h>
#include <res_material.h>
#include <res_resource_manager.h>
#include <unordered_map>
#include <gfx_graphics_device.h>
#include <gfx_buffer.h>
#include <math_raycasting.h>

namespace pge
{
    using game_StaticMeshId                         = unsigned;
    static const unsigned game_StaticMeshId_Invalid = -1;
    class game_StaticMeshManager {
        struct StaticMeshEntity {
            game_Entity         entity;
            const res_Mesh*     mesh;
            const res_Material* material;
        };
        std::vector<StaticMeshEntity>                      m_meshes;
        std::unordered_map<game_Entity, game_StaticMeshId> m_entityMap;

        struct CBTransforms {
            math_Mat4x4 modelMatrix;
            math_Mat4x4 viewMatrix;
            math_Mat4x4 projMatrix;
        } m_cbTransformsData;
        gfx_GraphicsDevice* m_graphicsDevice;
        gfx_ConstantBuffer  m_cbTransforms;

        res_ResourceManager* m_resources;

    public:
        game_StaticMeshManager(size_t               capacity,
                               gfx_GraphicsAdapter* graphicsAdapter,
                               gfx_GraphicsDevice*  graphicsDevice,
                               res_ResourceManager* resources);

        void CreateStaticMeshes(const game_Entity* entities, size_t numEntities, game_StaticMeshId* destBuf);
        void DestroyStaticMesh(const game_StaticMeshId& id);
        void GarbageCollect(const game_EntityManager& entityManager);

        bool              HasStaticMesh(const game_Entity& entity) const;
        game_StaticMeshId GetStaticMeshId(const game_Entity& entity) const;

        void SetMesh(const game_StaticMeshId& id, const res_Mesh* mesh);
        void SetMaterial(const game_StaticMeshId& id, const res_Material* material);

        game_Entity     GetEntity(const game_StaticMeshId& id) const;
        const res_Mesh* GetMesh(const game_StaticMeshId& id) const;

        void              DrawStaticMeshes(const game_TransformManager& tm, const math_Mat4x4& view, const math_Mat4x4& proj);
        game_StaticMeshId GetRaycastStaticMesh(const game_TransformManager& tm, const math_Ray& ray, const math_Mat4x4& viewProj) const;

        friend std::ostream& operator<<(std::ostream& os, const game_StaticMeshManager& sm);
        friend std::istream& operator>>(std::istream& is, game_StaticMeshManager& sm);
    };
} // namespace pge

#endif