#ifndef PGE_RESOURCE_RES_SKELETON_H
#define PGE_RESOURCE_RES_SKELETON_H

#include <memory>
#include <unordered_map>
#include <anim_skeleton.h>
#include <iostream>
#include <fstream>

namespace pge
{
    struct res_Bone {
        std::string name;
        math_Mat4x4 transform;
        int         parent;
    };

    inline std::istream&
    operator>>(std::istream& is, res_Bone& bone)
    {
        unsigned nameSz = bone.name.size();
        is.read((char*)&nameSz, sizeof(nameSz));
        std::unique_ptr<char[]> name(new char[nameSz + 1]);
        is.read((char*)&name[0], nameSz);
        name[nameSz] = 0;
        bone.name = name.get();

        is.read((char*)&bone.transform, sizeof(bone.transform));
        is.read((char*)&bone.parent, sizeof(bone.parent));
        return is;
    }

    inline std::ostream&
    operator<<(std::ostream& os, res_Bone& bone)
    {
        unsigned nameSz = bone.name.size();
        os.write((const char*)&nameSz, sizeof(nameSz));
        os.write((const char*)bone.name.c_str(), nameSz);
        os.write((const char*)&bone.transform, sizeof(bone.transform));
        os.write((const char*)&bone.parent, sizeof(bone.parent));
        return os;
    }


    class res_Skeleton {
        std::string                    m_path;
        std::unique_ptr<anim_Skeleton> m_skeleton;

        void
        MakeSkeleton(res_Bone* bones, unsigned numBones)
        {
            std::vector<anim_SkeletonBone> sbones;
            sbones.reserve(numBones);
            for (unsigned i = 0; i < numBones; ++i) {
                const res_Bone&   resBone = bones[i];
                anim_SkeletonBone bone;
                strcpy_s(bone.name, sizeof(bone.name), resBone.name.c_str());
                bone.localTransform = resBone.transform;
                bone.worldTransform = resBone.transform;
                bone.parentIdx = resBone.parent;
                sbones.emplace_back(bone);
            }
            m_skeleton = std::make_unique<anim_Skeleton>(&sbones[0], numBones);
        }

    public:
        res_Skeleton(res_Bone* bones, unsigned numBones)
            : m_path("<from-memory>")
        {
            MakeSkeleton(bones, numBones);
        }

        res_Skeleton(const char* path)
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
        Write(std::ostream& os) const
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
        GetPath() const
        {
            return m_path.c_str();
        }

        const anim_Skeleton*
        GetSkeleton() const
        {
            return m_skeleton.get();
        }
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