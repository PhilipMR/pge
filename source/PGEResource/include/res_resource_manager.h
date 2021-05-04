#ifndef PGE_RESOURCE_RESOURCE_MANAGER_H
#define PGE_RESOURCE_RESOURCE_MANAGER_H

#include "res_effect.h"
#include "res_texture2d.h"
#include "res_material.h"
#include "res_mesh.h"

namespace pge
{
    class res_ResourceManager {
        res_EffectCache    m_effects;
        res_Texture2DCache m_textures;
        res_MaterialCache  m_materials;
        res_MeshCache      m_meshes;

    public:
        explicit res_ResourceManager(gfx_GraphicsAdapter* graphicsAdapter);

        const res_Effect*    GetEffect(const char* path);
        const res_Texture2D* GetTexture(const char* path);
        const res_Material*  GetMaterial(const char* path);
        const res_Mesh*      GetMesh(const char* path);
    };
} // namespace pge

#endif