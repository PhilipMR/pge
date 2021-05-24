#include "../include/res_resource_manager.h"

namespace pge
{
    res_ResourceManager::res_ResourceManager(gfx_GraphicsAdapter* graphicsAdapter)
        : m_effects(graphicsAdapter)
        , m_textures(graphicsAdapter)
        , m_materials(graphicsAdapter, &m_effects, &m_textures)
        , m_meshes(graphicsAdapter)
        , m_skeletons()
        , m_skeletonAnimations()
        , m_animConfigs(&m_skeletons, &m_skeletonAnimations)
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

    const res_Skeleton*
    res_ResourceManager::GetSkeleton(const char* path)
    {
        return m_skeletons.Load(path);
    }

    const res_SkeletonAnimation*
    res_ResourceManager::GetSkeletonAnimation(const char* path)
    {
        return m_skeletonAnimations.Load(path);
    }

    const res_AnimatorConfig*
    res_ResourceManager::GetAnimatorConfig(const char* path)
    {
        return m_animConfigs.Load(path);
    }
} // namespace pge