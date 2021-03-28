#ifndef PGE_MATH_MATH_QUAT_H
#define PGE_MATH_MATH_QUAT_H

#include "math_vec4.h"
#include "math_constants.h"
#include <diag_assert.h>

namespace pge
{
    struct math_Quat {
        float w;
        union {
            struct {
                float x, y, z;
            };
            math_Vec3 xyz;
        };

        constexpr math_Quat()
            : w(1)
            , x(0)
            , y(0)
            , z(0)
        {}

        constexpr math_Quat(float w, float x, float y, float z)
            : w(w)
            , x(x)
            , y(y)
            , z(z)
        {}

        constexpr math_Quat(float w, const math_Vec3& xyz)
            : w(w)
            , xyz(xyz)
        {}

        constexpr float
        operator[](size_t idx) const
        {
            switch (idx) {
                case 0: return w;
                case 1: return x;
                case 2: return y;
                case 3: return z;
                default: diag_CrashAndBurn("Quaternion subscript was out of range.");
            }
            return 0.0f;
        }

        constexpr float&
        operator[](size_t idx)
        {
            switch (idx) {
                case 0: return w;
                case 1: return x;
                case 2: return y;
                case 3: return z;
            }
            diag_CrashAndBurn("Quaternion subscript was out of range.");
            return x;
        }
    };

    constexpr bool
    operator==(const math_Quat& lhs, const math_Quat& rhs)
    {
        for (size_t i = 0; i < 4; ++i) {
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return false;
        }
        return true;
    }

    constexpr bool
    operator!=(const math_Quat& lhs, const math_Quat& rhs)
    {
        for (size_t i = 0; i < 4; ++i) {
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return true;
        }
        return false;
    }

    constexpr math_Quat
    operator-(const math_Quat& quat)
    {
        return math_Quat(-quat.w, -quat.xyz);
    }

    constexpr math_Quat
    operator+(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(lhs.w + rhs.w, lhs.xyz + rhs.xyz);
    }

    constexpr math_Quat
    operator-(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(lhs.w - rhs.w, lhs.xyz - rhs.xyz);
    }

    constexpr math_Quat
    operator*(float scalar, const math_Quat& quat)
    {
        return math_Quat(quat.w * scalar, quat.xyz * scalar);
    }

    constexpr math_Quat
    operator*(const math_Quat& quat, float scalar)
    {
        return scalar * quat;
    }

    constexpr math_Quat
    operator*(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(rhs.w * lhs.w - math_Dot(lhs.xyz, rhs.xyz), rhs.w * lhs.xyz + lhs.w * rhs.xyz + math_Cross(lhs.xyz, rhs.xyz));
    }

    constexpr math_Vec4
    operator*(const math_Quat& quat, const math_Vec4& vec)
    {
        math_Quat vecQuat(vec.w, vec.xyz);
        math_Quat res = quat * vecQuat;
        return math_Vec4(res.xyz, res.w);
    }

    constexpr math_Vec4
    operator*(const math_Vec4& vec, const math_Quat& quat)
    {
        math_Quat vecQuat(vec.w, vec.xyz);
        math_Quat res = vecQuat * quat;
        return math_Vec4(res.xyz, res.w);
    }

    constexpr math_Quat
    operator/(const math_Quat& quat, float scalar)
    {
        diag_Assert(scalar != static_cast<float>(0));
        return (1.0f / scalar) * quat;
    }

    constexpr math_Quat&
    operator+=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs + rhs;
        return lhs;
    }

    constexpr math_Quat
    operator-=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs - rhs;
        return lhs;
    }

    constexpr math_Quat&
    operator*=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs * rhs;
        return lhs;
    }

    constexpr math_Quat&
    operator*=(math_Quat& quat, float scalar)
    {
        quat = quat * scalar;
        return quat;
    }

    constexpr math_Quat&
    operator/=(math_Quat& quat, float scalar)
    {
        diag_Assert(scalar != static_cast<float>(0));
        quat = quat / scalar;
        return quat;
    }


    constexpr math_Quat
    math_Conjugate(const math_Quat& quat)
    {
        return math_Quat(quat.w, -quat.x, -quat.y, -quat.z);
    }

    constexpr float
    math_Dot(const math_Quat& lhs, const math_Quat& rhs)
    {
        return lhs.w * rhs.w + math_Dot(lhs.xyz, rhs.xyz);
    }

    constexpr float
    math_LengthSquared(const math_Quat& quat)
    {
        return math_Dot(quat, quat);
    }

    inline float
    math_Length(const math_Quat& quat)
    {
        return sqrtf(static_cast<float>(math_LengthSquared(quat)));
    }

    inline math_Quat
    math_Normalize(const math_Quat& quat)
    {
        float len = math_Length(quat);
        diag_Assert(len != 0.0f);
        return math_Quat(quat.w / len, quat.xyz / len);
    }

    constexpr math_Quat
    math_Invert(const math_Quat& quat)
    {
        float f = math_Dot(quat, quat);
        return math_Conjugate(quat) / f;
    }

    constexpr math_Quat
    math_Lerp(const math_Quat& from, const math_Quat& to, float alpha)
    {
        return (1 - alpha) * from + alpha * to;
    }

    constexpr math_Quat
    math_QuatFromAxisAngle(const math_Vec3& axis, float degrees)
    {
        const float rad   = math_DegToRad(degrees);
        math_Vec3   naxis = axis;
        if (math_LengthSquared(naxis) > 0)
            naxis = math_Normalize(naxis);
        return math_Quat(static_cast<float>(cosf(rad * 0.5f)), static_cast<float>(sinf(rad * 0.5f)) * naxis);
    }

    inline math_Quat
    math_QuatFromEulerAngles(const math_Vec3& euler)
    {
        const float roll  = euler.x;
        const float pitch = euler.y;
        const float yaw   = euler.z;

        double cy = cosf(yaw * 0.5);
        double sy = sinf(yaw * 0.5);
        double cp = cosf(pitch * 0.5);
        double sp = sinf(pitch * 0.5);
        double cr = cosf(roll * 0.5);
        double sr = sinf(roll * 0.5);

        math_Quat q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;

        return q;
    }

    inline math_Vec3
    math_EulerAnglesFromQuaternion(const math_Quat& quat)
    {
        struct EulerAngles {
            float roll, pitch, yaw;
        } angles;
        const math_Quat& q = quat;

        // roll (x-axis rotation)
        double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
        double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
        angles.roll = std::atan2(sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        double sinp = 2 * (q.w * q.y - q.z * q.x);
        if (std::abs(sinp) >= 1)
            angles.pitch = std::copysign(math_PI / 2, sinp); // use 90 degrees if out of range
        else
            angles.pitch = std::asin(sinp);

        // yaw (z-axis rotation)
        double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
        double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
        angles.yaw = std::atan2(siny_cosp, cosy_cosp);

        return math_Vec3(angles.roll, angles.pitch, angles.yaw);
    }

    constexpr math_Vec4
    math_Rotate(const math_Vec4& vec, const math_Quat& quat)
    {
        math_Quat qinv   = math_Invert(quat);
        math_Quat result = quat * math_Quat(vec.w, vec.xyz) * qinv;
        return math_Vec4(result.xyz, result.w);
    }

    constexpr math_Vec3
    math_Rotate(const math_Vec3& vec, const math_Quat& quat)
    {
        math_Quat qinv = math_Invert(quat);
        return (quat * math_Vec4(vec, 1) * qinv).xyz;
    }

    constexpr math_Quat
    math_Rotate(const math_Quat& quat, const math_Vec3& axis, float degrees)
    {
        math_Quat rotation = math_QuatFromAxisAngle(axis, degrees);
        return rotation * quat;
    }
} // namespace pge

#endif