#include "../include/res_animator.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace pge
{
    res_AnimatorConfig::res_AnimatorConfig(const char* path, res_SkeletonCache* skeletonCache, res_SkeletonAnimationCache* animationCache)
        : m_path(path)
    {
        std::ifstream is(path);
        core_Assert(is.is_open());
        nlohmann::json json;
        is >> json;
        is.close();

        std::string          skelPath = json["skeleton"];
        const anim_Skeleton* skeleton = skeletonCache->Load(skelPath.c_str())->GetSkeleton();

        std::vector<anim_AnimationState> states;
        auto                             jstates = json["animation_states"];
        for (auto& jstate : jstates) {
            std::string name     = jstate["name"];
            std::string animpath = jstate["path"];
            bool        looping  = jstate["looping"];

            const anim_SkeletonAnimation* anim = animationCache->Load(animpath.c_str())->GetAnimation();
            states.push_back(anim_AnimationState(name.c_str(), anim, looping));
        }

        std::vector<anim_TransitionParam> transitions;
        auto                              jtransitions = json["transitions"];
        for (auto& jtrans : jtransitions) {
            std::string from     = jtrans["from"];
            std::string to       = jtrans["to"];
            std::string trigger  = jtrans["trigger"];
            float       duration = jtrans["duration"];

            transitions.push_back(anim_TransitionParam(from.c_str(), to.c_str(), trigger.c_str(), duration));
        }

        m_config.Initialize(skeleton, &states[0], states.size(), &transitions[0], transitions.size());
    }

    const char*
    res_AnimatorConfig::GetPath() const
    {
        return m_path.c_str();
    }

    const anim_AnimatorConfig*
    res_AnimatorConfig::GetConfig() const
    {
        return &m_config;
    }


    res_AnimatorConfigCache::res_AnimatorConfigCache(res_SkeletonCache* skeletonCache, res_SkeletonAnimationCache* animationCache)
        : m_skeletonCache(skeletonCache)
        , m_animationCache(animationCache)
    {}

    res_AnimatorConfig*
    res_AnimatorConfigCache::Load(const char* path)
    {
        auto it = m_configMap.find(path);
        if (it == m_configMap.end()) {
            m_configMap.emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple(path, m_skeletonCache, m_animationCache));
        }
        return &m_configMap.at(path);
    }
} // namespace pge