#ifndef PGE_GRAPHICS_GFX_RENDER_TARGET_H
#define PGE_GRAPHICS_GFX_RENDER_TARGET_H

#include <memory>
#include <d3d11.h>

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_RenderTarget {
        class gfx_RenderTargetImpl;
        std::unique_ptr<gfx_RenderTargetImpl> m_impl;

    public:
        gfx_RenderTarget(gfx_GraphicsAdapter* graphicsAdapter, unsigned width, unsigned height, bool hasDepth);
        ~gfx_RenderTarget();
        void Clear();
        void Bind() const;
        void BindTexture(unsigned slot) const;
        void* GetNativeTexture();
    };

    void gfx_RenderTarget_BindMainRTV(gfx_GraphicsAdapter* graphicsAdapter);
}

#endif