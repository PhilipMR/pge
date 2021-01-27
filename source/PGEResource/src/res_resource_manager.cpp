#include "../include/res_resource_manager.h"

namespace pge
{
    res_ResourceManager::res_ResourceManager(gfx_GraphicsAdapter* graphicsAdapter)
        : m_effects(graphicsAdapter)
        , m_textures(graphicsAdapter)
        , m_materials(graphicsAdapter, &m_effects, &m_textures)
        , m_meshes(graphicsAdapter)
    {}

    const res_Effect*
    res_ResourceManager::GetEffect(const char* path)
    {
        return m_effects.Load(path);
    }

    const res_Texture2D*
    res_ResourceManager::GetTexture(const char* path)
    {
        return m_textures.Load(path);
    }

    const res_Material*
    res_ResourceManager::GetMaterial(const char* path)
    {
        return m_materials.Load(path);
    }

    const res_Mesh*
    res_ResourceManager::GetMesh(const char* path)
    {
        return m_meshes.Load(path);
    }
} // namespace pge