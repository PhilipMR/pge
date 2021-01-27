#ifndef PGE_GRAPHICS_GFX_GRAPHICS_ADAPTER_H
#define PGE_GRAPHICS_GFX_GRAPHICS_ADAPTER_H

#include <memory>

namespace pge
{
    class gfx_GraphicsAdapter {
    protected:
        class gfx_GraphicsAdapterImpl;
        std::unique_ptr<gfx_GraphicsAdapterImpl> m_impl;
        gfx_GraphicsAdapter();
        ~gfx_GraphicsAdapter();
    };
} // namespace pge

#endif