#include "../include/gfx_graphics_device.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <diag_assert.h>

namespace pge
{
    struct gfx_GraphicsDevice::gfx_GraphicsDeviceImpl {
        IDXGISwapChain*         m_swapChain;
        ID3D11DeviceContext*    m_deviceContext;
        ID3D11RenderTargetView* m_mainRTV;
        ID3D11DepthStencilView* m_depthStencil;
        float                   m_clearColor[4];
    };
    gfx_GraphicsDevice::gfx_GraphicsDevice(gfx_GraphicsAdapter* adapter)
        : m_impl(new gfx_GraphicsDeviceImpl)
    {
        auto adapterD3D11       = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(adapter);
        m_impl->m_swapChain     = adapterD3D11->GetSwapChain();
        m_impl->m_deviceContext = adapterD3D11->GetDeviceContext();
        m_impl->m_mainRTV       = adapterD3D11->GetMainRTV();
        m_impl->m_depthStencil  = adapterD3D11->GetDepthStencil();
    }

    gfx_GraphicsDevice::~gfx_GraphicsDevice() = default;

    void
    gfx_GraphicsDevice::Clear()
    {
        m_impl->m_deviceContext->ClearRenderTargetView(m_impl->m_mainRTV, m_impl->m_clearColor);
        m_impl->m_deviceContext->ClearDepthStencilView(m_impl->m_depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    }

    void
    gfx_GraphicsDevice::Present()
    {
        m_impl->m_swapChain->Present(1, 0);
    }

    static D3D11_PRIMITIVE_TOPOLOGY
    GetPrimitiveType3D11(gfx_PrimitiveType type)
    {
        switch (type) {
            case gfx_PrimitiveType::POINTLIST: return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            case gfx_PrimitiveType::LINELIST: return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case gfx_PrimitiveType::TRIANGLELIST: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case gfx_PrimitiveType::TRIANGLESTRIP: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            default: diag_CrashAndBurn("Unhandled case for gfx_PrimitiveType."); break;
        }
        return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }

    void
    gfx_GraphicsDevice::Draw(gfx_PrimitiveType primitive, unsigned first, unsigned count)
    {
        m_impl->m_deviceContext->IASetPrimitiveTopology(GetPrimitiveType3D11(primitive));
        m_impl->m_deviceContext->Draw(count, first);
    }

    void
    gfx_GraphicsDevice::DrawIndexed(gfx_PrimitiveType primitive, unsigned first, unsigned count)
    {
        m_impl->m_deviceContext->IASetPrimitiveTopology(GetPrimitiveType3D11(primitive));
        m_impl->m_deviceContext->DrawIndexed(count, first, 0);
    }

    void
    gfx_GraphicsDevice::SetClearColor(float r, float g, float b, float a)
    {
        m_impl->m_clearColor[0] = r;
        m_impl->m_clearColor[1] = g;
        m_impl->m_clearColor[2] = b;
        m_impl->m_clearColor[3] = a;
    }
} // namespace pge