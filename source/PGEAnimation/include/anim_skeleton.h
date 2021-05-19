#ifndef PGE_ANIMATION_ANIM_SKELETON_H
#define PGE_ANIMATION_ANIM_SKELETON_H

#include <math_mat4x4.h>

namespace pge
{
    struct anim_SkeletonBone {
        char        name[64];
        math_Mat4x4 localTransform;
        math_Mat4x4 worldTransform;
        int         parentIdx;
    };

    struct anim_KeyVec3 {
        math_Vec3 value;
        double    time;
    };

    struct anim_KeyQuat {
        math_Quat value;
        double    time;
    };

    class anim_SkeletonAnimationChannel {
        char  m_boneName[64];
        void* m_buffer;

        unsigned      m_numPosKeys;
        anim_KeyVec3* m_positionKeys;

        unsigned      m_numScaleKeys;
        anim_KeyVec3* m_scaleKeys;

        unsigned      m_numRotKeys;
        anim_KeyQuat* m_rotationKeys;

    public:
        anim_SkeletonAnimationChannel() = default;
        anim_SkeletonAnimationChannel(const char*   boneName,
                                      anim_KeyVec3* posKeys,
                                      unsigned      numPosKeys,
                                      anim_KeyVec3* scaleKeys,
                                      unsigned      numScaleKeys,
                                      anim_KeyQuat* rotKeys,
                                      unsigned      numRotKeys);
        ~anim_SkeletonAnimationChannel();

        const char* GetBoneName() const;

        unsigned      GetPositionKeyCount() const;
        anim_KeyVec3* GetPositionKeys() const;

        unsigned      GetScaleKeyCount() const;
        anim_KeyVec3* GetScaleKeys() const;

        unsigned      GetRotationKeyCount() const;
        anim_KeyQuat* GetRotationKeys() const;
    };

    class anim_SkeletonAnimation {
        double                                     m_duration;
        std::vector<anim_SkeletonAnimationChannel> m_channels;

    public:
        anim_SkeletonAnimation(double duration, const anim_SkeletonAnimationChannel* channels, unsigned numChannels);
        double                               GetDuration() const;
        const anim_SkeletonAnimationChannel* GetChannels() const;
        unsigned                             GetChannelCount() const;
    };

    class anim_Skeleton {
        std::vector<anim_SkeletonBone> m_bones;

        anim_SkeletonBone* FindBone(const char* name);

    public:
        anim_Skeleton(anim_SkeletonBone* bones, unsigned numBones);

        anim_Skeleton(const anim_Skeleton& rhs)
            : m_bones(rhs.m_bones)
        {}

        void Transform();
        void Animate(const anim_SkeletonAnimation& animation, double time);
        void Animate(const anim_SkeletonAnimation& from, double fromTime, const anim_SkeletonAnimation& to, double toTime, float factor);

        size_t                   GetBoneCount() const;
        const anim_SkeletonBone& GetBone(size_t index) const;
    };

    void
    anim_DebugDraw_Skeleton(const anim_Skeleton& skeleton, const math_Mat4x4& modelMatrix, const math_Vec3& color, float lineWidth, bool hasDepth);

} // namespace pge

#endif