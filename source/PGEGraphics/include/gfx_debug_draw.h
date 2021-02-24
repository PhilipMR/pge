#ifndef PGE_GRAPHICS_GFX_DEBUG_DRAW_H
#define PGE_GRAPHICS_GFX_DEBUG_DRAW_H

#include <math_mat4x4.h>

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_GraphicsDevice;

    void gfx_DebugDraw_Initialize(gfx_GraphicsAdapter* graphicsAdapter, gfx_GraphicsDevice* graphicsDevice);
    void gfx_DebugDraw_Shutdown();
    void gfx_DebugDraw_SetView(const math_Mat4x4& viewMatrix);
    void gfx_DebugDraw_SetProjection(const math_Mat4x4& projectionMatrix);
    void gfx_DebugDraw_Point(const math_Vec3& position, const math_Vec3& color = math_Vec3(1.f, 1.f, 1.f), float size = 0.01f, bool depthTest = true);
    void gfx_DebugDraw_Line(const math_Vec3& begin,
                            const math_Vec3& end,
                            const math_Vec3& color     = math_Vec3(1.f, 1.f, 1.f),
                            float            width     = 0.1f,
                            bool             depthTest = true);
    void gfx_DebugDraw_Box(const math_Vec3& min,
                           const math_Vec3& max,
                           const math_Vec3& color     = math_Vec3(1.f, 1.f, 1.f),
                           float            lineWidth = 0.075f,
                           bool             depthTest = true);
    void gfx_DebugDraw_Flush();
} // namespace pge

#endif