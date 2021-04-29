#include "../include/gfx_sampler.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <core_assert.h>

namespace pge
{
    struct gfx_Sampler::gfx_SamplerImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11SamplerState*  m_samplerState;
    };

    gfx_Sampler::gfx_Sampler(gfx_GraphicsAdapter* graphicsAdapter)
        : m_impl(new gfx_SamplerImpl)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        m_impl->m_deviceContext                        = graphicsAdapterD3D11->GetDeviceContext();

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU           = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV           = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW           = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.ComparisonFunc     = D3D11_COMPARISON_LESS_EQUAL;
        samplerDesc.MinLOD             = 0.0f;
        samplerDesc.MaxLOD             = D3D11_FLOAT32_MAX;
        HRESULT result                 = graphicsAdapterD3D11->GetDevice()->CreateSamplerState(&samplerDesc, &m_impl->m_samplerState);
        core_Assert(SUCCEEDED(result));
    }

    gfx_Sampler::~gfx_Sampler()
    {
        m_impl->m_samplerState->Release();
    }

    void
    gfx_Sampler::Bind(unsigned slot) const
    {
        m_impl->m_deviceContext->PSSetSamplers(slot, 1, &m_impl->m_samplerState);
    }
} // namespace pge