#ifndef PGE_GRAPHICS_GFX_GRAPHICS_DEVICE_H
#define PGE_GRAPHICS_GFX_GRAPHICS_DEVICE_H

#include <memory>

namespace pge
{
    class gfx_GraphicsAdapter;

    enum class gfx_PrimitiveType
    {
        POINTLIST,
        LINELIST,
        TRIANGLELIST,
        TRIANGLESTRIP
    };

    enum class gfx_RasterizerState
    {
        SOLID_CULL_BACK,
        SOLID_CULL_FRONT,
        WIREFRAME,
        NUM_RASTERIZER_STATES
    };

    class gfx_GraphicsDevice {
        class gfx_GraphicsDeviceImpl;
        std::unique_ptr<gfx_GraphicsDeviceImpl> m_impl;

    public:
        explicit gfx_GraphicsDevice(gfx_GraphicsAdapter* adapter);
        ~gfx_GraphicsDevice();

        void Present();
        void Draw(gfx_PrimitiveType primitive, unsigned first, unsigned count);
        void DrawIndexed(gfx_PrimitiveType primitive, unsigned first, unsigned count);
        void SetViewport(float x, float y, float width, float height);
        void SetRasterizerState(const gfx_RasterizerState& state);
        gfx_RasterizerState GetRasterizerState() const;
    };
} // namespace pge

#endif