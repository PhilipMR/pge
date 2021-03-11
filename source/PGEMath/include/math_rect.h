#ifndef PGE_MATH_MATH_RECT_H
#define PGE_MATH_MATH_RECT_H

#include "math_vec2.h"

namespace pge
{
    struct math_Rect {
        float x, y, w, h;

        math_Rect()
            : x(0)
            , y(0)
            , w(0)
            , h(0)
        {}

        math_Rect(float x, float y, float w, float h)
            : x(x)
            , y(y)
            , w(w)
            , h(h)
        {}

        math_Rect(const math_Vec2& position, const math_Vec2& size)
            : x(position.x)
            , y(position.y)
            , w(size.x)
            , h(size.y)
        {}

        constexpr bool
        Intersects(const math_Vec2& point) const
        {
            return point.x >= x && point.x <= x+w
                && point.y >= y && point.y <= y+h;
        }

        constexpr bool
        Intersects(const math_Rect& other) const
        {
            return Intersects(math_Vec2(   other.x,           other.y))
                || Intersects(math_Vec2(   other.x,        other.y+other.h))
                || Intersects(math_Vec2(other.x+other.w,   other.y))
                || Intersects(math_Vec2(other.x+other.w,other.y+other.h));
        }
    };
} // namespace pge

#endif