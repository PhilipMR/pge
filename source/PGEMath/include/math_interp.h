#ifndef PGE_MATH_MATH_INTERP_H
#define PGE_MATH_MATH_INTERP_H

namespace pge
{
    template <typename T>
    T math_Lerp(const T& from, const T& to, float factor)
    {
        return (1-factor) * from + factor * to;
    }
}

#endif