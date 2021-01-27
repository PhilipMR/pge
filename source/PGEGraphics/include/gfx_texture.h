#ifndef PGE_GRAPHICS_GFX_TEXTURE_H
#define PGE_GRAPHICS_GFX_TEXTURE_H

#include <memory>

namespace pge
{
    enum class gfx_PixelFormat {
        R8G8B8A8_UNORM
    };

    class gfx_GraphicsAdapter;
    class gfx_Texture2D {
        class gfx_Texture2DImpl;
        std::unique_ptr<gfx_Texture2DImpl> m_impl;

    public:
        gfx_Texture2D(gfx_GraphicsAdapter* graphicsAdapter, gfx_PixelFormat format, unsigned width, unsigned height, void* data);
        ~gfx_Texture2D();
        void Bind(unsigned slot) const;
    };
} // namespace pge

#endif