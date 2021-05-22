#ifndef PGE_RESOURCE_RES_SKELETON_H
#define PGE_RESOURCE_RES_SKELETON_H

#include <anim_skeleton.h>

#include <memory>
#include <unordered_map>
#include <iostream>

namespace pge
{
    struct res_Bone {
        std::string name;
        math_Mat4x4 transform;
        int         parent;
    };

    class res_Skeleton {
        std::string                    m_path;
        std::unique_ptr<anim_Skeleton> m_skeleton;

        void MakeSkeleton(res_Bone* bones, unsigned numBones);

    public:
        res_Skeleton(res_Bone* bones, unsigned numBones);
        res_Skeleton(const char* path);

        void                 Write(std::ostream& os) const;
        const char*          GetPath() const;
        const anim_Skeleton* GetSkeleton() const;
    };

    class res_SkeletonCache {
        std::unordered_map<std::string, res_Skeleton> m_skelMap;

    public:
        res_SkeletonCache();
        res_Skeleton* Load(const char* path);
    };

    class res_SkeletonAnimation {
        std::string                             m_path;
        std::unique_ptr<anim_SkeletonAnimation> m_animation;

    public:
        res_SkeletonAnimation(std::unique_ptr<anim_SkeletonAnimation> animation);
        res_SkeletonAnimation(const char* name, double duration, const anim_SkeletonAnimationChannel* channels, unsigned numChannels);
        res_SkeletonAnimation(const char* path);

        void                          Write(std::ostream& os) const;
        const char*                   GetPath() const;
        const anim_SkeletonAnimation* GetAnimation() const;
    };

    class res_SkeletonAnimationCache {
        std::unordered_map<std::string, res_SkeletonAnimation> m_animMap;

    public:
        res_SkeletonAnimationCache();
        res_SkeletonAnimation* Load(const char* path);
    };
} // namespace pge

#endif