#include "../include/res_skeleton.h"
#include <fstream>

namespace pge
{
    // =========================================
    // res_Bone
    // =========================================
    static std::istream&
    operator>>(std::istream& is, res_Bone& bone)
    {
        unsigned nameSz = 0;
        is.read((char*)&nameSz, sizeof(nameSz));
        std::unique_ptr<char[]> name(new char[nameSz + 1]);
        is.read((char*)&name[0], nameSz);
        name[nameSz] = 0;
        bone.name    = name.get();

        is.read((char*)&bone.transform, sizeof(bone.transform));
        is.read((char*)&bone.parent, sizeof(bone.parent));
        return is;
    }

    static std::ostream&
    operator<<(std::ostream& os, const res_Bone& bone)
    {
        unsigned nameSz = bone.name.size();
        os.write((const char*)&nameSz, sizeof(nameSz));
        os.write((const char*)bone.name.c_str(), nameSz);
        os.write((const char*)&bone.transform, sizeof(bone.transform));
        os.write((const char*)&bone.parent, sizeof(bone.parent));
        return os;
    }


    // =========================================
    // res_Skeleton
    // =========================================
    void
    res_Skeleton::MakeSkeleton(res_Bone* bones, unsigned numBones)
    {
        std::vector<anim_SkeletonBone> sbones;
        sbones.reserve(numBones);
        for (unsigned i = 0; i < numBones; ++i) {
            const res_Bone&   resBone = bones[i];
            anim_SkeletonBone bone;
            strcpy_s(bone.name, sizeof(bone.name), resBone.name.c_str());
            bone.localTransform = resBone.transform;
            bone.worldTransform = resBone.transform;
            bone.parentIdx      = resBone.parent;
            sbones.emplace_back(bone);
        }
        m_skeleton = std::make_unique<anim_Skeleton>(&sbones[0], numBones);
    }

    res_Skeleton::res_Skeleton(res_Bone* bones, unsigned numBones)
        : m_path("<from-memory>")
    {
        MakeSkeleton(bones, numBones);
    }

    res_Skeleton::res_Skeleton(const char* path)
        : m_path(path)
    {
        std::ifstream skelFile(path, std::ios::binary);

        unsigned numBones = 0;
        skelFile.read((char*)&numBones, sizeof(numBones));

        std::vector<res_Bone> resBones;
        resBones.reserve(numBones);
        for (unsigned i = 0; i < numBones; ++i) {
            res_Bone bone;
            skelFile >> bone;
            resBones.emplace_back(bone);
        }

        MakeSkeleton(&resBones[0], numBones);
    }

    void
    res_Skeleton::Write(std::ostream& os) const
    {
        unsigned numBones = m_skeleton->GetBoneCount();
        os.write((char*)&numBones, sizeof(numBones));

        for (unsigned i = 0; i < numBones; ++i) {
            const anim_SkeletonBone& skelBone = m_skeleton->GetBone(i);
            res_Bone                 bone;
            bone.name   = skelBone.name;
            bone.parent = -1;
            if (skelBone.parentIdx != -1) {
                unsigned parIdx = 0;
                for (; strcmp(m_skeleton->GetBone(parIdx).name, m_skeleton->GetBone(skelBone.parentIdx).name) != 0; parIdx++) {
                    core_Assert(parIdx < numBones);
                }
                bone.parent = parIdx;
            }
            bone.transform = skelBone.localTransform;
            os << bone;
        }
    }

    const char*
    res_Skeleton::GetPath() const
    {
        return m_path.c_str();
    }

    const anim_Skeleton*
    res_Skeleton::GetSkeleton() const
    {
        return m_skeleton.get();
    }


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


    // =========================================
    // res_SkeletonAnimation
    // =========================================
    res_SkeletonAnimation::res_SkeletonAnimation(std::unique_ptr<anim_SkeletonAnimation> animation)
        : m_path("<from-memory>")
        , m_animation(std::move(animation))
    {}

    res_SkeletonAnimation::res_SkeletonAnimation(const char*                          name,
                                                 double                               duration,
                                                 const anim_SkeletonAnimationChannel* channels,
                                                 unsigned                             numChannels)
        : m_path("<from-memory>")
        , m_animation(std::make_unique<anim_SkeletonAnimation>(name, duration, channels, numChannels))
    {}

    res_SkeletonAnimation::res_SkeletonAnimation(const char* path)
        : m_path(path)
    {
        std::ifstream is(path, std::ios::binary);
        core_Assert(is.is_open());
        m_animation = std::make_unique<anim_SkeletonAnimation>(is);
    }

    void
    res_SkeletonAnimation::Write(std::ostream& os) const
    {
        os << *m_animation.get();
    }

    const char*
    res_SkeletonAnimation::GetPath() const
    {
        return m_path.c_str();
    }

    const anim_SkeletonAnimation*
    res_SkeletonAnimation::GetAnimation() const
    {
        return m_animation.get();
    }


    res_SkeletonAnimation*
    res_SkeletonAnimationCache::Load(const char* path)
    {
        auto it = m_animMap.find(path);
        if (it == m_animMap.end()) {
            m_animMap.emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple(path));
        }
        return &m_animMap.at(path);
    }
} // namespace pge