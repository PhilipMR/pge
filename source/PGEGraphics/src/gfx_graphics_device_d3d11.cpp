#include "../include/gfx_graphics_device.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <core_assert.h>

namespace pge
{
    struct gfx_GraphicsDevice::gfx_GraphicsDeviceImpl {
        IDXGISwapChain*        m_swapChain;
        ID3D11DeviceContext*   m_deviceContext;
        ID3D11RasterizerState* m_rasterizerStates[(size_t)gfx_RasterizerState::NUM_RASTERIZER_STATES];
        gfx_RasterizerState    m_rasterizerCurrentState;
    };

    static ID3D11RasterizerState*
    CreateRasterizerState(ID3D11Device* device, const gfx_RasterizerState& state)
    {
        D3D11_RASTERIZER_DESC rasterizerDesc;
        rasterizerDesc.FillMode              = (state == gfx_RasterizerState::WIREFRAME) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
        rasterizerDesc.CullMode              = (state == gfx_RasterizerState::SOLID_CULL_FRONT) ? D3D11_CULL_FRONT : D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = true;
        rasterizerDesc.DepthBias             = D3D11_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp        = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias  = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable       = true;
        rasterizerDesc.ScissorEnable         = false;
        rasterizerDesc.MultisampleEnable     = true;
        rasterizerDesc.AntialiasedLineEnable = false;

        ID3D11RasterizerState* rasterizerState;
        HRESULT                result = device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
        core_Assert(SUCCEEDED(result));
        return rasterizerState;
    }


    gfx_GraphicsDevice::gfx_GraphicsDevice(gfx_GraphicsAdapter* adapter)
        : m_impl(new gfx_GraphicsDeviceImpl)
    {
        auto adapterD3D11       = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(adapter);
        m_impl->m_swapChain     = adapterD3D11->GetSwapChain();
        m_impl->m_deviceContext = adapterD3D11->GetDeviceContext();
        ID3D11Device* device    = adapterD3D11->GetDevice();
        for (size_t i = 0; i < (size_t)gfx_RasterizerState::NUM_RASTERIZER_STATES; ++i) {
            m_impl->m_rasterizerStates[i] = CreateRasterizerState(device, (gfx_RasterizerState)i);
        }
        m_impl->m_rasterizerCurrentState = gfx_RasterizerState::SOLID_CULL_BACK;
    }

    gfx_GraphicsDevice::~gfx_GraphicsDevice()
    {
        for (size_t i = 0; i < (size_t)gfx_RasterizerState::NUM_RASTERIZER_STATES; ++i) {
            m_impl->m_rasterizerStates[i]->Release();
        }
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
            default: core_CrashAndBurn("Unhandled case for gfx_PrimitiveType."); break;
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
    gfx_GraphicsDevice::SetViewport(float x, float y, float width, float height)
    {
        D3D11_VIEWPORT viewport = {0};
        viewport.TopLeftX       = x;
        viewport.TopLeftY       = y;
        viewport.Width          = width;
        viewport.Height         = height;
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        m_impl->m_deviceContext->RSSetViewports(1, &viewport);
    }

    void
    gfx_GraphicsDevice::SetRasterizerState(const gfx_RasterizerState& state)
    {
        m_impl->m_deviceContext->RSSetState(m_impl->m_rasterizerStates[(size_t)state]);
        m_impl->m_rasterizerCurrentState = state;
    }

    gfx_RasterizerState
    gfx_GraphicsDevice::GetRasterizerState() const
    {
        return m_impl->m_rasterizerCurrentState;
    }
} // namespace pge