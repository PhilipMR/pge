#ifndef PGE_ANIMATION_ANIM_EASING_H
#define PGE_ANIMATION_ANIM_EASING_H

namespace pge
{
    template <typename T>
    T anim_Lerp(const T& from, const T& to, float factor)
    {
        return (1-factor) * from + factor * to;
    }
}

#endif