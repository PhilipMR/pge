#ifndef PGE_MATH_MATH_VEC2_H
#define PGE_MATH_MATH_VEC2_H

namespace pge
{
    struct math_Vec2 {
        union {
            struct {
                float x, y;
            };
            float xy[2];
        };

        inline static math_Vec2
        Zero()
        {
            return math_Vec2(0, 0);
        }

        inline static math_Vec2
        One()
        {
            return math_Vec2(1, 1);
        }

        inline math_Vec2()
            : x(0)
            , y(0)
        {}

        inline math_Vec2(float x, float y)
            : x(x)
            , y(y)
        {}

        inline const float
        operator[](size_t index)
        {
            return xy[index];
        }

        inline const float&
        operator[](size_t index) const
        {
            return xy[index];
        }

        inline math_Vec2&
        operator+=(const math_Vec2& rhs)
        {
            this->x += rhs.x;
            this->y += rhs.y;
            return *this;
        }

        inline math_Vec2&
        operator-=(const math_Vec2& rhs)
        {
            this->x -= rhs.x;
            this->y -= rhs.y;
            return *this;
        }

        inline math_Vec2&
        operator*=(float scalar)
        {
            this->x *= scalar;
            this->y *= scalar;
            return *this;
        }

        inline math_Vec2&
        operator/=(float scalar)
        {
            this->x /= scalar;
            this->y /= scalar;
            return *this;
        }
    };

    inline math_Vec2
    operator+(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        return math_Vec2(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    inline math_Vec2
    operator-(const math_Vec2& lhs, const math_Vec2& rhs)
    {
        return math_Vec2(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    inline math_Vec2
    operator-(const math_Vec2& lhs)
    {
        return math_Vec2(-lhs.x, -lhs.y);
    }

    inline math_Vec2
    operator*(const math_Vec2& vec, float scalar)
    {
        return math_Vec2(vec.x * scalar, vec.y * scalar);
    }

    inline math_Vec2
    operator*(float scalar, const math_Vec2& vec)
    {
        return vec * scalar;
    }

    inline math_Vec2
    operator/(const math_Vec2& vec, float scalar)
    {
        return vec * (1.0f / scalar);
    }

    inline float
    math_Dot(const math_Vec2& left, const math_Vec2& right)
    {
        return left.x * right.x + left.y * right.y;
    }

    inline float
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