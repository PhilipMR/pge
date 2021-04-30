#ifndef PGE_MATH_MATH_AABB_H
#define PGE_MATH_MATH_AABB_H

#include "math_mat4x4.h"

namespace pge
{
    struct math_AABB {
        union {
            struct {
                math_Vec3 min;
                math_Vec3 max;
            };
            math_Vec3 bounds[2];
        };

        constexpr math_AABB()
            : min(math_Vec3::Zero())
            , max(math_Vec3::Zero())
        {}

        constexpr math_AABB(const math_Vec3& min, const math_Vec3& max)
            : min(min)
            , max(max)
        {}
    };

    inline math_AABB
    math_CreateAABB(const char* positionBuffer, size_t numPositions, size_t stride, size_t offset)
    {
        const char* vtxPtr = positionBuffer;

        const float floatMax = std::numeric_limits<float>::max();
        const float floatMin = -floatMax;

        math_Vec3 min(floatMax, floatMax, floatMax);
        math_Vec3 max(floatMin, floatMin, floatMin);
        for (size_t i = 0; i < numPositions; ++i) {
            const auto* pos = reinterpret_cast<const float*>(vtxPtr + i * stride + offset);
            for (size_t j = 0; j < 3; ++j) {
                if (pos[j] < min[j])
                    min[j] = pos[j];
                if (pos[j] > max[j])
                    max[j] = pos[j];
            }
        }
        return math_AABB(min, max);
    }

    inline math_AABB
    math_TransformAABB(const math_AABB& aabb, const math_Mat4x4& xform)
    {
        math_Vec3 diff     = aabb.max - aabb.min;
        math_Vec4 points[] = {
            xform * math_Vec4(aabb.min + math_Vec3(0, 0, 0), 1),
            xform * math_Vec4(aabb.min + math_Vec3(diff.x, 0, 0), 1),
            xform * math_Vec4(aabb.min + math_Vec3(0, diff.y, 0), 1),
            xform * math_Vec4(aabb.min + math_Vec3(diff.x, diff.y, 0), 1),
            xform * math_Vec4(aabb.min + math_Vec3(0, 0, diff.z), 1),
            xform * math_Vec4(aabb.min + math_Vec3(diff.x, 0, diff.z), 1),
            xform * math_Vec4(aabb.min + math_Vec3(0, diff.y, diff.z), 1),
            xform * math_Vec4(aabb.min + math_Vec3(diff.x, diff.y, diff.z), 1),
        };
        return math_CreateAABB((const char*)points, 8, sizeof(math_Vec4), 0);
    }

} // namespace pge

#endif