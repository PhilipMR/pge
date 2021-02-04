#ifndef PGE_MATH_MATH_VEC2_H
#define PGE_MATH_MATH_VEC2_H

#include "math_constants.h"
#include <cmath>

namespace pge
{
    struct math_Vec2 {
        union {
            struct {
                float x, y;
            };
            float xy[2];
        };

        constexpr static math_Vec2
        Zero()
        {
            return math_Vec2(0, 0);
        }

        constexpr static math_Vec2
        One()
        {
            return math_Vec2(1, 1);
        }

        constexpr math_Vec2()
            : x(0)
            , y(0)
        {}

        constexpr math_Vec2(float x, float y)
            : x(x)
            , y(y)
        {}

        constexpr const float
        operator[](size_t index)
        {
            return xy[index];
        }

        constexpr const float&
        operator[](size_t index) const
        {
            return xy[index];
        }

        constexpr math_Vec2&
        operator+=(const math_Vec2& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            return *this;
        }

        constexpr math_Vec2&
        operator-=(const math_Vec2& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            return *this;
        }

        constexpr math_Vec2&
        operator*=(float scalar)
        {
            this->x *= scalar;
            this->y *= scalar;
            return *this;
        }

        constexpr math_Vec2&
        operator/=(float scalar)
        {
            this->x /= scalar;
            this->y /= scalar;
            return *this;
        }
    };

    constexpr bool
    operator==(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        for (size_t i = 0; i < 2; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return false;
        return true;
    }

    constexpr bool
    operator!=(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        for (size_t i = 0; i < 2; ++i)
            if (!math_FloatEqual(lhs[i], rhs[i]))
                return true;
        return false;
    }

    constexpr math_Vec2
    operator+(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        return math_Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    constexpr math_Vec2
    operator-(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        return math_Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    constexpr math_Vec2
    operator-(const math_Vec2& lhs)
    {
        return math_Vec2(-lhs.x, -lhs.y);
    }

    constexpr math_Vec2
    operator*(const math_Vec2& vec, float scalar)
    {
        return math_Vec2(vec.x * scalar, vec.y * scalar);
    }

    constexpr math_Vec2
    operator*(float scalar, const math_Vec2& vec)
    {
        return vec * scalar;
    }

    constexpr math_Vec2
    operator/(const math_Vec2& vec, float scalar)
    {
        return vec * (1.0f / scalar);
    }

    constexpr float
    math_Dot(const math_Vec2& left, const math_Vec2& right)
    {
        return left.x * right.x + left.y * right.y;
    }

    constexpr float
    math_LengthSquared(const math_Vec2& vec)
    {
        return math_Dot(vec, vec);
    }

    inline float
    math_Length(const math_Vec2& vec)
    {
        return sqrtf(math_LengthSquared(vec));
    }

    inline math_Vec2
    math_Normalize(const math_Vec2& vec)
    {
        float invLen = 1.0f / math_Length(vec);
        return math_Vec2(vec.x * invLen, vec.y * invLen);
    }
} // namespace pge

#endif