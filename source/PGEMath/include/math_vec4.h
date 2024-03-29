#ifndef PGE_MATH_MATH_VEC4_H
#define PGE_MATH_MATH_VEC4_H

#include "math_vec3.h"

namespace pge
{
    struct math_Vec4i {
        union {
            struct { int x, y, z, w; };
            int xyzw[4];
        };

        constexpr math_Vec4i()
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        {}

        constexpr math_Vec4i(int x, int y, int z, int w)
            : x(x)
            , y(y)
            , z(z)
            , w(w)
        {}

        constexpr int&
        operator[](size_t index)
        {
            return xyzw[index];
        }

        constexpr const int&
        operator[](size_t index) const
        {
            return xyzw[index];
        }
    };

    struct math_Vec4 {
        union {
            struct {
                union {
                    math_Vec3 xyz;
                    struct {
                        float x, y, z;
                    };
                };
                float w;
            };
            float xyzw[4];
        };

        constexpr static math_Vec4
        Zero()
        {
            return math_Vec4(0, 0, 0, 0);
        };

        constexpr static math_Vec4
        One()
        {
            return math_Vec4(1, 1, 1, 1);
        };

        constexpr math_Vec4()
            : x(0)
            , y(0)
            , z(0)
            , w(0)
        {}

        constexpr math_Vec4(float x, float y, float z, float w)
            : x(x)
            , y(y)
            , z(z)
            , w(w)
        {}

        constexpr math_Vec4(const math_Vec3& vec, float w)
            : x(vec.x)
            , y(vec.y)
            , z(vec.z)
            , w(w)
        {}

        constexpr float&
        operator[](size_t index)
        {
            return xyzw[index];
        }

        constexpr const float&
        operator[](size_t index) const
        {
            return xyzw[index];
        }

        constexpr math_Vec4&
        operator+=(const math_Vec4& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            this->z += rhs.z;
            this->w += rhs.w;
            return *this;
        }

        constexpr math_Vec4&
        operator-=(const math_Vec4& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            this->z -= rhs.z;
            this->w -= rhs.w;
            return *this;
        }

        constexpr math_Vec4&
        operator*=(float scalar)
        {
            this->x *= scalar;
            this->y *= scalar;
            this->z *= scalar;
            this->w *= scalar;
            return *this;
        }

        constexpr math_Vec4&
        operator/=(float scalar)
        {
            this->x /= scalar;
            this->y /= scalar;
            this->z /= scalar;
            this->w /= scalar;
            return *this;
        }
    };

    constexpr bool
    operator==(const math_Vec4& lhs, const math_Vec4& rhs)
    {
        for (size_t i = 0; i < 4; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return false;
        return true;
    }

    constexpr bool
    operator!=(const math_Vec4& lhs, const math_Vec4& rhs)
    {
        for (size_t i = 0; i < 4; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return true;
        return false;
    }

    constexpr math_Vec4
    operator+(const math_Vec4& lhs, const math_Vec4& rhs)
    {
        return math_Vec4(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
    }

    constexpr math_Vec4
    operator-(const math_Vec4& lhs, const math_Vec4& rhs)
    {
        return math_Vec4(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
    }

    constexpr math_Vec4
    operator-(const math_Vec4& lhs)
    {
        return math_Vec4(-lhs.x, -lhs.y, -lhs.z, -lhs.w);
    }

    constexpr math_Vec4
    operator*(const math_Vec4& vec, float scalar)
    {
        return math_Vec4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
    }

    constexpr math_Vec4
    operator*(float scalar, const math_Vec4& vec)
    {
        return vec * scalar;
    }

    constexpr math_Vec4
    operator/(const math_Vec4& vec, float scalar)
    {
        return vec * (1.0f / scalar);
    }

    constexpr float
    math_Dot(const math_Vec4& left, const math_Vec4& right)
    {
        return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
    }

    constexpr float
    math_LengthSquared(const math_Vec4& vec)
    {
        return math_Dot(vec, vec);
    }

    inline float
    math_Length(const math_Vec4& vec)
    {
        return sqrtf(math_LengthSquared(vec));
    }

    inline math_Vec4
    math_Normalize(const math_Vec4& vec)
    {
        float invLen = 1.0f / math_Length(vec);
        return math_Vec4(vec.x * invLen, vec.y * invLen, vec.z * invLen, vec.w * invLen);
    }
} // namespace pge

#endif