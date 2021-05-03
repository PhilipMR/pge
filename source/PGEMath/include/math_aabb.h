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

        inline math_AABB(const char* buffer, size_t numPoints, size_t stride, size_t offset)
        {
            const float floatMax = std::numeric_limits<float>::max();
            const float floatMin = -floatMax;

            math_Vec3 min(floatMax, floatMax, floatMax);
            math_Vec3 max(floatMin, floatMin, floatMin);

            for (size_t i = 0; i < numPoints; ++i) {
                const auto* pos = reinterpret_cast<const float*>(buffer + i * stride + offset);

                for (size_t j = 0; j < 3; ++j) {
                    const auto smin = static_cast<int>(pos[j] < min[j]);
                    const auto gmax = static_cast<int>(pos[j] > max[j]);

                    min[j] = smin * pos[j] + (1 - smin) * min[j];
                    max[j] = gmax * pos[j] + (1 - gmax) * max[j];
                }
            }

            this->min = min;
            this->max = max;
        }

        inline math_AABB(const math_Vec3* points, size_t numPoints)
            : math_AABB(reinterpret_cast<const char*>(points), numPoints, sizeof(math_Vec3), 0)
        {}

        inline math_AABB(const math_Vec4* points, size_t numPoints)
            : math_AABB(reinterpret_cast<const char*>(points), numPoints, sizeof(math_Vec4), 0)
        {}
    };

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
        return math_AABB(points, 8);
    }

} // namespace pge

#endif