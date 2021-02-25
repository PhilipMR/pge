#ifndef PGE_MATH_MATH_RAYCASTING_H
#define PGE_MATH_MATH_RAYCASTING_H

#include "math_mat4x4.h"
#include "math_aabb.h"
#include <limits>

namespace pge
{
    struct math_Ray {
        constexpr math_Ray() {}
        constexpr math_Ray(const math_Vec3& origin, const math_Vec3& direction)
            : origin(origin)
            , direction(direction)
        {}
        math_Vec3 origin;
        math_Vec3 direction;
    };

    constexpr bool
    math_Raycast_IntersectsAABB(const math_Ray& ray, const math_AABB& aabb, float* distance)
    {
        if (distance != nullptr)
            *distance = std::numeric_limits<float>::max();

        if (math_LengthSquared(ray.direction) == 0)
            return false;

        int  sign[3] = {};
        math_Vec3 invdir{1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z};
        sign[0] = (invdir.x < 0);
        sign[1] = (invdir.y < 0);
        sign[2] = (invdir.z < 0);

        float tmin  = (aabb.bounds[sign[0]].x - ray.origin.x) * invdir.x;
        float tmax  = (aabb.bounds[1 - sign[0]].x - ray.origin.x) * invdir.x;
        float tymin = (aabb.bounds[sign[1]].y - ray.origin.y) * invdir.y;
        float tymax = (aabb.bounds[1 - sign[1]].y - ray.origin.y) * invdir.y;

        if ((tmin > tymax) || (tymin > tmax))
            return false;

        if (tymin > tmin)
            tmin = tymin;
        if (tymax < tmax)
            tmax = tymax;

        float tzmin = (aabb.bounds[sign[2]].z - ray.origin.z) * invdir.z;
        float tzmax = (aabb.bounds[1 - sign[2]].z - ray.origin.z) * invdir.z;

        if ((tmin > tzmax) || (tzmin > tmax))
            return false;

        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;

        if (distance != nullptr)
            *distance = tmin;
        return true;
    }

    constexpr math_Vec3
    math_Raycast_Unproject(const math_Vec3& pixel, const math_Vec2& windowSize, const math_Mat4x4& viewProjInv)
    {
        math_Vec4 in(pixel.x / windowSize.x * 2.0f - 1.0f, pixel.y / windowSize.y * 2.0f - 1.0f, pixel.z * -2.0f, 1.0f);
        math_Vec4 out = viewProjInv * in;
        diag_Assert(out.w != 0.0f);
        return out.xyz / out.w;
    }

    inline math_Ray
    math_Raycast_RayFromPixel(const math_Vec2& pixel, const math_Vec2& windowSize, const math_Mat4x4& viewProj)
    {
        math_Mat4x4 viewProjInv;
        diag_Verify(math_Invert(viewProj, &viewProjInv));
        math_Vec3 origin = math_Raycast_Unproject(math_Vec3(pixel.x, windowSize.y - pixel.y, 0.0f), windowSize, viewProjInv);
        math_Vec3 tmp    = math_Raycast_Unproject(math_Vec3(pixel.x, windowSize.y - pixel.y, 1.0f), windowSize, viewProjInv);
        math_Vec3 dir    = math_Normalize(origin - tmp);
        return math_Ray(origin, dir);
    }
} // namespace pge

#endif