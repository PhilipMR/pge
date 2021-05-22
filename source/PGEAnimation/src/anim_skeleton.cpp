#include "../include/anim_skeleton.h"
#include "../include/anim_easing.h"

#include <core_assert.h>
#include <gfx_debug_draw.h>
#include <memory>
#include <iostream>

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
    void
    anim_SkeletonAnimationChannel::MakeSkeletonAnimationChannel(const char*         boneName,
                                                                const anim_KeyVec3* posKeys,
                                                                unsigned            numPosKeys,
                                                                const anim_KeyVec3* scaleKeys,
                                                                unsigned            numScaleKeys,
                                                                const anim_KeyQuat* rotKeys,
                                                                unsigned            numRotKeys)
    {
        core_Assert(strlen(boneName) <= sizeof(m_boneName));
        m_numPosKeys   = numPosKeys;
        m_numScaleKeys = numScaleKeys;
        m_numRotKeys   = numRotKeys;

        strcpy_s(m_boneName, boneName);

        if (m_buffer != nullptr) {
            free(m_buffer);
        }
        m_buffer       = malloc(sizeof(anim_KeyVec3) * numPosKeys + sizeof(anim_KeyVec3) * numScaleKeys + sizeof(anim_KeyQuat) * numRotKeys);
        m_positionKeys = reinterpret_cast<anim_KeyVec3*>(m_buffer);
        m_scaleKeys    = reinterpret_cast<anim_KeyVec3*>(m_positionKeys + numPosKeys);
        m_rotationKeys = reinterpret_cast<anim_KeyQuat*>(m_scaleKeys + numScaleKeys);

        memcpy(m_positionKeys, posKeys, sizeof(anim_KeyVec3) * numPosKeys);
        memcpy(m_scaleKeys, scaleKeys, sizeof(anim_KeyVec3) * numScaleKeys);
        memcpy(m_rotationKeys, rotKeys, sizeof(anim_KeyQuat) * numRotKeys);
    }


    anim_SkeletonAnimationChannel::anim_SkeletonAnimationChannel()
        : m_buffer(nullptr)
    {}

    anim_SkeletonAnimationChannel::anim_SkeletonAnimationChannel(const anim_SkeletonAnimationChannel& rhs)
    {
        MakeSkeletonAnimationChannel(rhs.m_boneName,
                                     rhs.m_positionKeys,
                                     rhs.m_numPosKeys,
                                     rhs.m_scaleKeys,
                                     rhs.m_numScaleKeys,
                                     rhs.m_rotationKeys,
                                     rhs.m_numRotKeys);
    }

    anim_SkeletonAnimationChannel::anim_SkeletonAnimationChannel(anim_SkeletonAnimationChannel&& rhs)
    {
        memcpy(this, &rhs, sizeof(rhs));
        memset(&rhs, 0, sizeof(rhs));
    }

    anim_SkeletonAnimationChannel&
    anim_SkeletonAnimationChannel::operator=(const anim_SkeletonAnimationChannel& rhs)
    {
        MakeSkeletonAnimationChannel(rhs.m_boneName,
                                     rhs.m_positionKeys,
                                     rhs.m_numPosKeys,
                                     rhs.m_scaleKeys,
                                     rhs.m_numScaleKeys,
                                     rhs.m_rotationKeys,
                                     rhs.m_numRotKeys);
        return *this;
    }

    anim_SkeletonAnimationChannel&
    anim_SkeletonAnimationChannel::operator=(anim_SkeletonAnimationChannel&& rhs)
    {
        memcpy(this, &rhs, sizeof(rhs));
        memset(&rhs, 0, sizeof(rhs));
        return *this;
    }

    anim_SkeletonAnimationChannel::anim_SkeletonAnimationChannel(const char*         boneName,
                                                                 const anim_KeyVec3* posKeys,
                                                                 unsigned            numPosKeys,
                                                                 const anim_KeyVec3* scaleKeys,
                                                                 unsigned            numScaleKeys,
                                                                 const anim_KeyQuat* rotKeys,
                                                                 unsigned            numRotKeys)
        : m_buffer(nullptr)
    {
        MakeSkeletonAnimationChannel(boneName, posKeys, numPosKeys, scaleKeys, numScaleKeys, rotKeys, numRotKeys);
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
    anim_SkeletonAnimation::anim_SkeletonAnimation(std::istream& is)
    {
        is >> *this;
    }

    anim_SkeletonAnimation::anim_SkeletonAnimation(const char*                          name,
                                                   double                               duration,
                                                   const anim_SkeletonAnimationChannel* channels,
                                                   unsigned                             numChannels)
        : m_name(name)
        , m_duration(duration)
    {
        m_channels.resize(numChannels);
        for (unsigned i = 0; i < numChannels; ++i) {
            m_channels[i] = channels[i];
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

    const char*
    anim_SkeletonAnimation::GetName() const
    {
        return m_name.c_str();
    }


    std::ostream&
    operator<<(std::ostream& os, const anim_SkeletonAnimationChannel& channel)
    {
        const char* boneName = channel.GetBoneName();
        unsigned    nameSz   = strlen(boneName);
        os.write((const char*)&nameSz, sizeof(nameSz));
        os.write((const char*)boneName, nameSz);

        unsigned numPosKeys = channel.GetPositionKeyCount();
        os.write((const char*)&numPosKeys, sizeof(numPosKeys));
        os.write((const char*)&channel.GetPositionKeys()[0], sizeof(channel.GetPositionKeys()[0]) * numPosKeys);

        unsigned numScaleKeys = channel.GetScaleKeyCount();
        os.write((const char*)&numScaleKeys, sizeof(numScaleKeys));
        os.write((const char*)&channel.GetScaleKeys()[0], sizeof(channel.GetScaleKeys()[0]) * numScaleKeys);

        unsigned numRotKeys = channel.GetRotationKeyCount();
        os.write((const char*)&numRotKeys, sizeof(numRotKeys));
        os.write((const char*)&channel.GetRotationKeys()[0], sizeof(channel.GetRotationKeys()[0]) * numRotKeys);

        return os;
    }

    std::istream&
    operator>>(std::istream& is, anim_SkeletonAnimationChannel& channel)
    {
        unsigned nameSz = 0;
        is.read((char*)&nameSz, sizeof(nameSz));
        std::unique_ptr<char[]> name(new char[nameSz + 1]);
        is.read((char*)&name[0], nameSz);
        name[nameSz] = 0;

        unsigned numPosKeys = 0;
        is.read((char*)&numPosKeys, sizeof(numPosKeys));
        std::unique_ptr<anim_KeyVec3[]> positionKeys(new anim_KeyVec3[numPosKeys]);
        is.read((char*)&positionKeys[0], sizeof(positionKeys[0]) * numPosKeys);

        unsigned numScaleKeys = 0;
        is.read((char*)&numScaleKeys, sizeof(numScaleKeys));
        std::unique_ptr<anim_KeyVec3[]> scaleKeys(new anim_KeyVec3[numScaleKeys]);
        is.read((char*)&scaleKeys[0], sizeof(scaleKeys[0]) * numScaleKeys);

        unsigned numRotKeys = 0;
        is.read((char*)&numRotKeys, sizeof(numRotKeys));
        std::unique_ptr<anim_KeyQuat[]> rotationKeys(new anim_KeyQuat[numScaleKeys]);
        is.read((char*)&rotationKeys[0], sizeof(rotationKeys[0]) * numRotKeys);

        channel
            .MakeSkeletonAnimationChannel(name.get(), positionKeys.get(), numPosKeys, scaleKeys.get(), numScaleKeys, rotationKeys.get(), numRotKeys);

        return is;
    }


    std::ostream&
    operator<<(std::ostream& os, const anim_SkeletonAnimation& animation)
    {
        unsigned nameLen = animation.m_name.size();
        os.write((const char*)&nameLen, sizeof(nameLen));
        os.write(animation.m_name.c_str(), nameLen);
        os.write((const char*)&animation.m_duration, sizeof(animation.m_duration));
        unsigned numChannels = animation.m_channels.size();
        os.write((const char*)&numChannels, sizeof(numChannels));
        for (const auto& c : animation.m_channels) {
            os << c;
        }
        return os;
    }

    std::istream&
    operator>>(std::istream& is, anim_SkeletonAnimation& animation)
    {
        unsigned nameSz = 0;
        is.read((char*)&nameSz, sizeof(nameSz));
        std::unique_ptr<char[]> name(new char[nameSz + 1]);
        is.read((char*)&name[0], nameSz);
        name[nameSz]     = 0;
        animation.m_name = name.get();

        is.read((char*)&animation.m_duration, sizeof(animation.m_duration));
        unsigned numChannels = 0;
        is.read((char*)&numChannels, sizeof(numChannels));
        animation.m_channels.resize(numChannels);
        for (unsigned i = 0; i < animation.m_channels.size(); ++i) {
            is >> animation.m_channels[i];
        }
        return is;
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
        m_bones.reserve(numBones);
        for (unsigned i = 0; i < numBones; ++i)
            m_bones.emplace_back(bones[i]);
    }

    void
    anim_Skeleton::Transform()
    {
        for (auto& bone : m_bones) {
            if (bone.parentIdx != -1) {
                bone.worldTransform = GetBone(bone.parentIdx).worldTransform * bone.localTransform;
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
        core_Assert(from.GetChannelCount() == to.GetChannelCount());
        unsigned                             channelCount = from.GetChannelCount();
        const anim_SkeletonAnimationChannel* fromChannels = from.GetChannels();
        const anim_SkeletonAnimationChannel* toChannels   = to.GetChannels();
        for (unsigned i = 0; i < channelCount; ++i) {
            const anim_SkeletonAnimationChannel& fromChannel = fromChannels[i];
            const anim_SkeletonAnimationChannel& toChannel   = toChannels[i];
            core_Assert(strcmp(fromChannel.GetBoneName(), toChannel.GetBoneName()) == 0);
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
    anim_DebugDraw_Skeleton(const anim_Skeleton& skeleton, const math_Mat4x4& modelMatrix, const math_Vec3& color, float lineWidth, bool hasDepth)
    {
        for (unsigned i = 0; i < skeleton.GetBoneCount(); ++i) {
            const anim_SkeletonBone& bone = skeleton.GetBone(i);

            math_Vec3 p1 = (modelMatrix * bone.worldTransform * math_Vec4(0.f, 0.f, 0.f, 1.f)).xyz;
            gfx_DebugDraw_Point(p1, color, lineWidth, hasDepth);

            if (bone.parentIdx != -1) {
                math_Mat4x4 parentTransform = skeleton.GetBone(bone.parentIdx).worldTransform;
                math_Vec3   p2              = (modelMatrix * parentTransform * math_Vec4(0.f, 0.f, 0.f, 1.f)).xyz;
                gfx_DebugDraw_Line(p1, p2, color, lineWidth, hasDepth);
            }
        }
    }
} // namespace pge