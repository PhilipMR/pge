#ifndef PGE_RESOURCE_RES_SKELETON_H
#define PGE_RESOURCE_RES_SKELETON_H

#include <unordered_map>

namespace pge
{
    class res_Skeleton {
    };

    class res_SkeletonCache {
        std::unordered_map<std::string, res_Skeleton> m_skelMap;

    public:
        res_SkeletonCache();
        res_Skeleton* Load(const char* path);
    };


    class res_SkeletonAnimation {
    };

    class res_SkeletonAnimationCache {
        std::unordered_map<std::string, res_SkeletonAnimation> m_animMap;

    public:
        res_SkeletonAnimationCache();
        res_SkeletonAnimation* Load(const char* path);
    };
} // namespace pge

#endif