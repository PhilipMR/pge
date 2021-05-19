#include "../include/res_skeleton.h"

namespace pge
{
    res_SkeletonCache::res_SkeletonCache() {}

    res_Skeleton*
    res_SkeletonCache::Load(const char* path)
    {
        auto it = m_skelMap.find(path);
        if (it == m_skelMap.end()) {
            m_skelMap.emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple(path));
        }
        return &m_skelMap.at(path);
    }

    res_SkeletonAnimationCache::res_SkeletonAnimationCache() {}

    res_SkeletonAnimation*
    res_SkeletonAnimationCache::Load(const char* path)
    {
        return nullptr;
    }
} // namespace pge