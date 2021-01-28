#ifndef PGE_MATH_MATH_VEC3_H
#define PGE_MATH_MATH_VEC3_H

#include "math_constants.h"
#include <diag_assert.h>

namespace pge
{
    struct math_Vec3 {
        union {
            struct {
                float x, y, z;
            };
            float xyz[3];
        };

        constexpr static math_Vec3
        Zero()
        {
            return math_Vec3(0, 0, 0);
        }
        constexpr static math_Vec3
        One()
        {
            return math_Vec3(1, 1, 1);
        }

        constexpr math_Vec3()
            : x(0)
            , y(0)
            , z(0)
        {}

        constexpr math_Vec3(float x, float y, float z)
            : x(x)
            , y(y)
            , z(z)
        {}

        constexpr const float
        operator[](size_t index)
        {
            return xyz[index];
        }

        constexpr const float&
        operator[](size_t index) const
        {
            return xyz[index];
        }

        constexpr math_Vec3&
        operator+=(const math_Vec3& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            this->z += rhs.z;
            return *this;
        }

        constexpr math_Vec3&
        operator-=(const math_Vec3& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            this->z -= rhs.z;
            return *this;
        }

        constexpr math_Vec3&
        operator*=(float scalar)
        {
            this->x *= scalar;
            this->y *= scalar;
            this->z *= scalar;
            return *this;
        }

        constexpr math_Vec3&
        operator/=(float scalar)
        {
            this->x /= scalar;
            this->y /= scalar;
            this->z /= scalar;
            return *this;
        }
    };

    constexpr bool
    operator==(const math_Vec3& lhs, const math_Vec3& rhs)
    {
        for (size_t i = 0; i < 3; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return false;
        return true;
    }

    constexpr bool
    operator!=(const math_Vec3& lhs, const math_Vec3& rhs)
    {
        for (size_t i = 0; i < 3; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return true;
        return false;
    }

    constexpr math_Vec3
    operator+(const math_Vec3& lhs, const math_Vec3& rhs)
    {
        return math_Vec3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
    }

    constexpr math_Vec3
    operator-(const math_Vec3& lhs, const math_Vec3& rhs)
    {
        return math_Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
    }

    constexpr math_Vec3
    operator-(const math_Vec3& lhs)
    {
        return math_Vec3(-lhs.x, -lhs.y, -lhs.z);
    }

    constexpr math_Vec3
    operator*(const math_Vec3& vec, float scalar)
    {
        return math_Vec3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
    }

    constexpr math_Vec3
    operator*(float scalar, const math_Vec3& vec)
    {
        return vec * scalar;
    }

    constexpr math_Vec3
    operator/(const math_Vec3& vec, float scalar)
    {
        return vec * (1.0f / scalar);
    }

    constexpr float
    math_Dot(const math_Vec3& left, const math_Vec3& right)
    {
        return left.x * right.x + left.y * right.y + left.z * right.z;
    }

    constexpr math_Vec3
    math_Cross(const math_Vec3& left, const math_Vec3& right)
    {
        return math_Vec3(left.y * right.z - left.z * right.y, left.z * right.x - left.x * right.z, left.x * right.y - left.y * right.x);
    }

    constexpr float
    math_LengthSquared(const math_Vec3& vec)
    {
        return math_Dot(vec, vec);
    }

    inline float
    math_Length(const math_Vec3& vec)
    {
        return sqrtf(math_LengthSquared(vec));
    }

    inline math_Vec3
    math_Normalize(const math_Vec3& vec)
    {
        float len = math_Length(vec);
        diag_Assert(len > 0);
        float invLen = 1.0f / len;
        return vec * invLen;
    }
} // namespace pge

#endif