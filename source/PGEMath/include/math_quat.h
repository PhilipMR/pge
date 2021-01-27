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
        }

        constexpr float&
        operator[](size_t idx)
        {
            switch (idx) {
                case 0: return w;
                case 1: return x;
                case 2: return y;
                case 3: return z;
                default: diag_CrashAndBurn("Quaternion subscript was out of range.");
            }
        }
    };


    constexpr bool
    operator==(const math_Quat& lhs, const math_Quat& rhs)
    {
        constexpr float epsilon = 0.001f;
        for (size_t i = 0; i < 4; ++i) {
            if (fabs(lhs[i] - rhs[i]) > epsilon)
                return false;
        }
        return true;
    }

    constexpr bool
    operator!=(const math_Quat& lhs, const math_Quat& rhs)
    {
        constexpr float epsilon = 0.001f;
        for (size_t i = 0; i < 4; ++i) {
            if (fabs(lhs[i] - rhs[i]) > epsilon)
                return true;
        }
        return false;
    }


    inline math_Quat
    operator-(const math_Quat& quat)
    {
        return math_Quat(-quat.w, -quat.xyz);
    }

    inline math_Quat
    operator+(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(lhs.w + rhs.w, lhs.xyz + rhs.xyz);
    }

    inline math_Quat
    operator-(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(lhs.w - rhs.w, lhs.xyz - rhs.xyz);
    }

    inline math_Quat
    operator*(float scalar, const math_Quat& quat)
    {
        return math_Quat(quat.w * scalar, quat.xyz * scalar);
    }

    inline math_Quat
    operator*(const math_Quat& quat, float scalar)
    {
        return scalar * quat;
    }

    inline math_Quat
    operator*(const math_Quat& lhs, const math_Quat& rhs)
    {
        return math_Quat(rhs.w * lhs.w - math_Dot(lhs.xyz, rhs.xyz), rhs.w * lhs.xyz + lhs.w * rhs.xyz + math_Cross(lhs.xyz, rhs.xyz));
    }

    inline math_Vec4
    operator*(const math_Quat& quat, const math_Vec4 vec)
    {
        math_Quat q(vec.w, vec.xyz);
        math_Quat r = quat * q;
        return math_Vec4(r.xyz, r.w);
    }

    inline math_Vec4
    operator*(const math_Vec4 vec, const math_Quat& quat)
    {
        return quat * vec;
    }

    inline math_Quat
    operator/(const math_Quat& quat, float scalar)
    {
        diag_Assert(scalar != static_cast<float>(0));
        return (1.0f / scalar) * quat;
    }


    inline math_Quat&
    operator+=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs + rhs;
        return lhs;
    }

    inline math_Quat
    operator-=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs - rhs;
        return lhs;
    }

    inline math_Quat&
    operator*=(math_Quat& lhs, const math_Quat& rhs)
    {
        lhs = lhs * rhs;
        return lhs;
    }

    inline math_Quat&
    operator*=(math_Quat& quat, float scalar)
    {
        quat = quat * scalar;
        return quat;
    }

    inline math_Quat&
    operator/=(math_Quat& quat, float scalar)
    {
        diag_Assert(scalar != static_cast<float>(0));
        quat = quat / scalar;
        return quat;
    }


    inline math_Quat
    math_Conjugate(const math_Quat& quat)
    {
        return math_Quat(quat.w, -quat.x, -quat.y, -quat.z);
    }

    inline float
    math_Dot(const math_Quat& lhs, const math_Quat& rhs)
    {
        return lhs.w * rhs.w + math_Dot(lhs.xyz, rhs.xyz);
    }

    inline float
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

    inline math_Quat
    math_Invert(const math_Quat& quat)
    {
        float f = quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w;
        return math_Quat(quat.w, quat.xyz) / f;
    }

    inline math_Quat
    math_Lerp(const math_Quat& from, const math_Quat& to, float alpha)
    {
        return (1 - alpha) * from + alpha * to;
    }

    // https://en.wikipedia.org/wiki/Slerp
    inline math_Quat
    math_Slerp(const math_Quat& left, const math_Quat& right, float alpha)
    {
        math_Quat q0 = left;
        math_Quat q1 = right;
        float     d  = math_Dot(q0, q1);

        if (d < 0) {
            d  = -d;
            q1 = -q1;
        }

        if ((1.0f - d) <= 0.0001f) {
            return math_Lerp(q0, q1, alpha);
        }
        float omega = acosf(d);
        float sinom = sinf(omega);
        float sclp  = sinf((1 - alpha) * omega) / sinom;
        float sclq  = sinf(alpha * omega) / sinom;
        return sclp * q0 + sclq * q1;
    }

    inline math_Quat
    math_QuaternionFromAxisAngle(const math_Vec3& axis, float degrees)
    {
        const float rad = math_DegToRad(degrees);
        return math_Quat(static_cast<float>(cosf(rad * 0.5f)), static_cast<float>(sinf(rad * 0.5f)) * math_Normalize(axis));
    }

    inline math_Vec4
    math_Rotate(const math_Vec4& vec, const math_Quat quat)
    {
        auto q    = math_Normalize(quat);
        auto qinv = math_Conjugate(q);
        return q * vec * qinv;
    }

    inline math_Quat
    math_Rotate(const math_Quat& quat, const math_Vec3& axis, float degrees)
    {
        math_Quat rotation = math_QuaternionFromAxisAngle(axis, degrees);
        return rotation * quat * math_Invert(rotation);
    }
} // namespace pge

#endif