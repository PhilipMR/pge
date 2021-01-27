#ifndef PGE_MATH_MATH_CONSTANTS_H
#define PGE_MATH_MATH_CONSTANTS_H

namespace pge
{
    constexpr float math_PI = 3.1415926535897932384626433832795f;

    constexpr float
    math_DegToRad(float deg)
    {
        return deg / 180.0f * math_PI;
    }

    constexpr float
    math_RadToDeg(float rad)
    {
        return rad / math_PI * 180.0f;
    }
} // namespace pge

#endif