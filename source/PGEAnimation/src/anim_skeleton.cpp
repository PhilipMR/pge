#include "../include/anim_skeleton.h"
#include "../include/anim_easing.h"

#include <core_assert.h>
#include <gfx_debug_draw.h>

namespace pge
{
    // ============================
    // Static helper functions
    // ============================
    template <typename TKey, typename TValue>
    static TValue
    Sample(const TKey* keys, unsigned keyCount, double time)
    {
        core_Assert(keyCount > 0);
        if (keyCount == 1) {
            return keys[0].value;
        }

        bool hasUpperBound = false;
        TKey lowerBound, upperBound;
        for (unsigned i = 0; i < keyCount; ++i) {
            const TKey& key = keys[i];
            if (math_FloatEqual(key.time, time)) {
                return key.value;
            } else if (key.time < time) {
                lowerBound = key;
            } else if (key.time > time) {
                upperBound    = key;
                hasUpperBound = true;
                break;
            }
        }
        if (!hasUpperBound) {
            return lowerBound.value;
        }

        double lowerOffset = time - lowerBound.time;
        double ratio       = lowerOffset / (upperBound.time - lowerBound.time);
        return anim_Lerp(lowerBound.value, upperBound.value, (float)ratio);
    }

    static void
    AnimateBone(anim_SkeletonBone* bone, const anim_SkeletonAnimationChannel& channel, double time)
    {
        math_Vec3 position   = Sample<anim_KeyVec3, math_Vec3>(channel.GetPositionKeys(), channel.GetPositionKeyCount(), time);
        math_Vec3 scale      = Sample<anim_KeyVec3, math_Vec3>(channel.GetScaleKeys(), channel.GetScaleKeyCount(), time);
        math_Quat rotation   = Sample<anim_KeyQuat, math_Quat>(channel.GetRotationKeys(), channel.GetRotationKeyCount(), time);
        bone->localTransform = math_CreateTransformMatrix(position, rotation, scale);
    }

    static void
    AnimateBone(anim_SkeletonBone*                   bone,
                const anim_SkeletonAnimationChannel& channelFrom,
                double                               timeFrom,
                const anim_SkeletonAnimationChannel& channelTo,
                double                               timeTo,
                float                                factor)
    {
        math_Vec3 posFrom   = Sample<anim_KeyVec3, math_Vec3>(channelFrom.GetPositionKeys(), channelFrom.GetPositionKeyCount(), timeFrom);
        math_Vec3 scaleFrom = Sample<anim_KeyVec3, math_Vec3>(channelFrom.GetScaleKeys(), channelFrom.GetScaleKeyCount(), timeFrom);
        math_Quat rotFrom   = Sample<anim_KeyQuat, math_Quat>(channelFrom.GetRotationKeys(), channelFrom.GetRotationKeyCount(), timeFrom);

        math_Vec3 posTo   = Sample<anim_KeyVec3, math_Vec3>(channelTo.GetPositionKeys(), channelTo.GetPositionKeyCount(), timeTo);
        math_Vec3 scaleTo = Sample<anim_KeyVec3, math_Vec3>(channelTo.GetScaleKeys(), channelTo.GetScaleKeyCount(), timeTo);
        math_Quat rotTo   = Sample<anim_KeyQuat, math_Quat>(channelTo.GetRotationKeys(), channelTo.GetRotationKeyCount(), timeTo);

        math_Vec3 position = anim_Lerp(posFrom, posTo, factor);
        math_Vec3 scale    = anim_Lerp(scaleFrom, scaleTo, factor);
        math_Quat rotation = anim_Lerp(rotFrom, rotTo, factor);

        bone->localTransform = math_CreateTransformMatrix(position, rotation, scale);
    }


    // ============================
    // anim_SkeletonAnimationChannel
    // ============================
    anim_SkeletonAnimationChannel::anim_SkeletonAnimationChannel(const char*   boneName,
                                                                 anim_KeyVec3* posKeys,
                                                                 unsigned      numPosKeys,
                                                                 anim_KeyVec3* scaleKeys,
                                                                 unsigned      numScaleKeys,
                                                                 anim_KeyQuat* rotKeys,
                                                                 unsigned      numRotKeys)
        : m_numPosKeys(numPosKeys)
        , m_numScaleKeys(numScaleKeys)
        , m_numRotKeys(numRotKeys)
    {
        core_Assert(strlen(boneName) <= sizeof(m_boneName));
        strcpy_s(m_boneName, boneName);

        m_buffer       = malloc(sizeof(anim_KeyVec3) * numPosKeys + sizeof(anim_KeyVec3) * numScaleKeys + sizeof(anim_KeyQuat) * numRotKeys);
        m_positionKeys = reinterpret_cast<anim_KeyVec3*>(m_buffer);
        m_scaleKeys    = reinterpret_cast<anim_KeyVec3*>(m_positionKeys + numPosKeys);
        m_rotationKeys = reinterpret_cast<anim_KeyQuat*>(m_scaleKeys + numScaleKeys);

        memcpy(m_positionKeys, posKeys, sizeof(anim_KeyVec3) * numPosKeys);
        memcpy(m_scaleKeys, scaleKeys, sizeof(anim_KeyVec3) * numScaleKeys);
        memcpy(m_rotationKeys, rotKeys, sizeof(anim_KeyQuat) * numRotKeys);
    }

    anim_SkeletonAnimationChannel::~anim_SkeletonAnimationChannel()
    {
        free(m_buffer);
    }

    const char*
    anim_SkeletonAnimationChannel::GetBoneName() const
    {
        return m_boneName;
    }

    unsigned
    anim_SkeletonAnimationChannel::GetPositionKeyCount() const
    {
        return m_numPosKeys;
    }

    anim_KeyVec3*
    anim_SkeletonAnimationChannel::GetPositionKeys() const
    {
        return &m_positionKeys[0];
    }

    unsigned
    anim_SkeletonAnimationChannel::GetScaleKeyCount() const
    {
        return m_numScaleKeys;
    }

    anim_KeyVec3*
    anim_SkeletonAnimationChannel::GetScaleKeys() const
    {
        return &m_scaleKeys[0];
    }

    unsigned
    anim_SkeletonAnimationChannel::GetRotationKeyCount() const
    {
        return m_numRotKeys;
    }

    anim_KeyQuat*
    anim_SkeletonAnimationChannel::GetRotationKeys() const
    {
        return &m_rotationKeys[0];
    }


    // ============================
    // anim_SkeletonAnimation
    // ============================
    anim_SkeletonAnimation::anim_SkeletonAnimation(double duration, const anim_SkeletonAnimationChannel* channels, unsigned numChannels)
        : m_duration(duration)
    {
        m_channels.resize(numChannels);
        for (unsigned i = 0; i < numChannels; ++i) {
            m_channels.emplace_back(channels[i]);
        }
    }

    double
    anim_SkeletonAnimation::GetDuration() const
    {
        return m_duration;
    }

    const anim_SkeletonAnimationChannel*
    anim_SkeletonAnimation::GetChannels() const
    {
        return &m_channels[0];
    }

    unsigned
    anim_SkeletonAnimation::GetChannelCount() const
    {
        return m_channels.size();
    }


    // ============================
    // anim_Skeleton
    // ============================
    anim_SkeletonBone*
    anim_Skeleton::FindBone(const char* name)
    {
        auto it = std::find_if(m_bones.begin(), m_bones.end(), [&](const anim_SkeletonBone& bone) { return strcmp(bone.name, name) == 0; });
        if (it == m_bones.end()) {
            return nullptr;
        } else {
            return &*it;
        }
    }

    anim_Skeleton::anim_Skeleton(anim_SkeletonBone* bones, unsigned numBones)
    {
        m_bones.resize(numBones);
        for (unsigned i = 0; i < numBones; ++i)
            m_bones.emplace_back(bones[i]);
    }

    void
    anim_Skeleton::Transform()
    {
        for (auto& bone : m_bones) {
            if (bone.parent != nullptr) {
                bone.worldTransform = bone.parent->worldTransform * bone.localTransform;
            } else {
                bone.worldTransform = bone.localTransform;
            }
        }
    }

    void
    anim_Skeleton::Animate(const anim_SkeletonAnimation& animation, double time)
    {
        unsigned                             channelCount = animation.GetChannelCount();
        const anim_SkeletonAnimationChannel* channels     = animation.GetChannels();
        for (unsigned i = 0; i < channelCount; ++i) {
            const anim_SkeletonAnimationChannel& channel = channels[i];

            anim_SkeletonBone* bone = FindBone(channel.GetBoneName());
            core_Assert(bone != nullptr);
            AnimateBone(bone, channel, time);
        }
    }

    void
    anim_Skeleton::Animate(const anim_SkeletonAnimation& from, double fromTime, const anim_SkeletonAnimation& to, double toTime, float factor)
    {
        core_Assert(from.channelCount == to.channelCount);
        unsigned                             channelCount = from.GetChannelCount();
        const anim_SkeletonAnimationChannel* fromChannels = from.GetChannels();
        const anim_SkeletonAnimationChannel* toChannels   = to.GetChannels();
        for (unsigned i = 0; i < channelCount; ++i) {
            const anim_SkeletonAnimationChannel& fromChannel = fromChannels[i];
            const anim_SkeletonAnimationChannel& toChannel   = toChannels[i];
            core_Assert(strcmp(fromChannel.boneName, toChannel.boneName) == 0);
            const char*        boneName = fromChannel.GetBoneName();
            anim_SkeletonBone* bone     = FindBone(boneName);
            core_Assert(bone != nullptr);
            AnimateBone(bone, fromChannel, fromTime, toChannel, toTime, factor);
        }
    }

    size_t
    anim_Skeleton::GetBoneCount() const
    {
        return m_bones.size();
    }

    const anim_SkeletonBone&
    anim_Skeleton::GetBone(size_t index) const
    {
        core_Assert(index < m_bones.size());
        return m_bones[index];
    }

    void
    anim_DebugDrawSkeleton(const anim_Skeleton& skeleton, const math_Mat4x4& modelMatrix, const math_Vec3& color, float lineWidth, bool hasDepth)
    {
        for (unsigned i = 0; i < skeleton.GetBoneCount(); ++i) {
            const anim_SkeletonBone& bone = skeleton.GetBone(i);

            math_Vec3 p1 = (modelMatrix * bone.worldTransform * math_Vec4(0.f, 0.f, 0.f, 1.f)).xyz;
            gfx_DebugDraw_Point(p1, color, lineWidth, hasDepth);

            if (bone.parent != nullptr) {
                math_Mat4x4 parentTransform = bone.parent->worldTransform;
                math_Vec3   p2              = (modelMatrix * parentTransform * math_Vec4(0.f, 0.f, 0.f, 1.f)).xyz;
                gfx_DebugDraw_Line(p1, p2, color, lineWidth, hasDepth);
            }
        }
    }
} // namespace pge