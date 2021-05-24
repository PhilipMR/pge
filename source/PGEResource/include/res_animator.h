#ifndef PGE_RESOURCE_RES_ANIMATOR_H
#define PGE_RESOURCE_RES_ANIMATOR_H

#include <anim_animator.h>
#include <string>
#include <unordered_map>

#include "res_skeleton.h"

namespace pge
{
    class res_AnimatorConfig {
        std::string         m_path;
        anim_AnimatorConfig m_config;

    public:
        res_AnimatorConfig(const char* path, res_SkeletonCache* skeletonCache, res_SkeletonAnimationCache* animationCache);

        const char*                GetPath() const;
        const anim_AnimatorConfig* GetConfig() const;
    };

    class res_AnimatorConfigCache {
        std::unordered_map<std::string, res_AnimatorConfig> m_configMap;
        res_SkeletonCache*                                  m_skeletonCache;
        res_SkeletonAnimationCache*                         m_animationCache;

    public:
        res_AnimatorConfigCache(res_SkeletonCache* skeletonCache, res_SkeletonAnimationCache* animationCache);
        res_AnimatorConfig* Load(const char* path);
    };
} // namespace pge

#endif