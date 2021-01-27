#ifndef PGE_GRAPHICS_GFX_GRAPHICS_ADAPTER_D3D11_H
#define PGE_GRAPHICS_GFX_GRAPHICS_ADAPTER_D3D11_H

#include "gfx_graphics_adapter.h"
#include <d3d11.h>

namespace pge
{
    class gfx_GraphicsAdapterD3D11 : public gfx_GraphicsAdapter {
    public:
        gfx_GraphicsAdapterD3D11(HWND hwnd, unsigned width, unsigned height);
        ~gfx_GraphicsAdapterD3D11();
        ID3D11Device*           GetDevice();
        ID3D11DeviceContext*    GetDeviceContext();
        ID3D11RenderTargetView* GetMainRTV();
        ID3D11DepthStencilView* GetDepthStencil();
        IDXGISwapChain*         GetSwapChain();
    };
} // namespace pge

#endif