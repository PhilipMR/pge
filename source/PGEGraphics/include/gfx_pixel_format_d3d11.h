#ifndef PGE_GRAPHICS_GFX_PIXEL_FORMAT_D3D11_H
#define PGE_GRAPHICS_GFX_PIXEL_FORMAT_D3D11_H

#include "gfx_texture.h"
#include <core_assert.h>
#include <dxgi.h>

namespace pge
{
    inline DXGI_FORMAT
    gfx_GetFormatDXGI(gfx_PixelFormat format)
    {
        switch (format) {
            case gfx_PixelFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case gfx_PixelFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case gfx_PixelFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
            default: core_CrashAndBurn("No mapping for gfx_PixelFormat.");
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    inline UINT
    gfx_GetFormatSizeBytes(gfx_PixelFormat format)
    {
        switch (format) {
            case gfx_PixelFormat::R8G8B8A8_UNORM: return 4;
            case gfx_PixelFormat::R32G32B32A32_FLOAT: return 16;
            case gfx_PixelFormat::R32_FLOAT: return 4;
            default: core_CrashAndBurn("No mapping for gfx_PixelFormat.");
        }
        return 0;
    }
}

#endif