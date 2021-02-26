#include "../include/gfx_render_target.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <diag_assert.h>
#include <d3d11.h>
#include <comdef.h>

namespace pge
{
    struct gfx_RenderTarget::gfx_RenderTargetImpl {
        unsigned                  m_width;
        unsigned                  m_height;
        ID3D11DeviceContext*      m_deviceContext;
        ID3D11Texture2D*          m_texture;
        ID3D11ShaderResourceView* m_srv;
        ID3D11RenderTargetView*   m_rtv;
        ID3D11DepthStencilView*   m_dsv;
    };

    gfx_RenderTarget::gfx_RenderTarget(gfx_GraphicsAdapter* graphicsAdapter, unsigned width, unsigned height, bool hasDepth, bool multisample)
        : m_impl(new gfx_RenderTargetImpl)
    {
        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();
        m_impl->m_width                    = width;
        m_impl->m_height                   = height;
        m_impl->m_deviceContext            = graphicsAdapterD3D11->GetDeviceContext();

        D3D11_TEXTURE2D_DESC textureDesc;
        textureDesc.Width              = width;
        textureDesc.Height             = height;
        textureDesc.MipLevels          = multisample ? 1 : 0;
        textureDesc.ArraySize          = 1;
        textureDesc.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
        textureDesc.SampleDesc.Count   = multisample ? 8 : 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage              = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags     = 0;
        textureDesc.MiscFlags          = 0;

        HRESULT result = device->CreateTexture2D(&textureDesc, nullptr, &m_impl->m_texture);
        diag_Assert(SUCCEEDED(result));

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                          = textureDesc.Format;
        srvDesc.ViewDimension                   = multisample ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels             = -1;
        srvDesc.Texture2D.MostDetailedMip       = 0;
        result                                  = device->CreateShaderResourceView(m_impl->m_texture, &srvDesc, &m_impl->m_srv);
        diag_Assert(SUCCEEDED(result));

        result = device->CreateRenderTargetView(m_impl->m_texture, nullptr, &m_impl->m_rtv);
        diag_Assert(SUCCEEDED(result));

        if (hasDepth) {
            // Create depth stencil view.
            D3D11_TEXTURE2D_DESC depthTextureDesc = {};
            depthTextureDesc.Width                = width;
            depthTextureDesc.Height               = height;
            depthTextureDesc.ArraySize            = 1;
            depthTextureDesc.SampleDesc           = textureDesc.SampleDesc;
            depthTextureDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthTextureDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
            depthTextureDesc.MipLevels = textureDesc.MipLevels;

            ID3D11Texture2D* depthTexture;
            result = device->CreateTexture2D(&depthTextureDesc, nullptr, &depthTexture);
            diag_Assert(SUCCEEDED(result));

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format                        = depthTextureDesc.Format;
            dsvDesc.ViewDimension                 = multisample ?  D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            result                                = device->CreateDepthStencilView(depthTexture, &dsvDesc, &m_impl->m_dsv);
            depthTexture->Release();
            diag_Assert(SUCCEEDED(result));
        } else {
            m_impl->m_dsv = nullptr;
        }
    }

    gfx_RenderTarget::~gfx_RenderTarget()
    {
        m_impl->m_rtv->Release();
        if (m_impl->m_dsv)
            m_impl->m_dsv->Release();
        m_impl->m_srv->Release();
        m_impl->m_texture->Release();
    }

    void
    gfx_RenderTarget::Clear()
    {
        float clearColor[] = {0, 0, 0, 1};
        m_impl->m_deviceContext->ClearRenderTargetView(m_impl->m_rtv, clearColor);
        if (m_impl->m_dsv) {
            m_impl->m_deviceContext->ClearDepthStencilView(m_impl->m_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        }
    }

    void
    gfx_RenderTarget::Bind() const
    {
        D3D11_VIEWPORT viewport = {0};
        viewport.Width          = m_impl->m_width;
        viewport.Height         = m_impl->m_height;
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        m_impl->m_deviceContext->RSSetViewports(1, &viewport);
        m_impl->m_deviceContext->OMSetRenderTargets(1, &m_impl->m_rtv, m_impl->m_dsv);
    }

    void
    gfx_RenderTarget::BindTexture(unsigned slot) const
    {
        m_impl->m_deviceContext->PSSetShaderResources(slot, 1, &m_impl->m_srv);
    }

    void*
    gfx_RenderTarget::GetNativeTexture() const
    {
        return m_impl->m_srv;
    }

    void
    gfx_RenderTarget_BindMainRTV(gfx_GraphicsAdapter* graphicsAdapter)
    {
        auto                    graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11RenderTargetView* rtv                  = graphicsAdapterD3D11->GetMainRTV();
        ID3D11DepthStencilView* dsv                  = graphicsAdapterD3D11->GetDepthStencil();
        graphicsAdapterD3D11->GetDeviceContext()->OMSetRenderTargets(1, &rtv, dsv);
    }

    void
    gfx_RenderTarget_ClearMainRTV(gfx_GraphicsAdapter* graphicsAdapter)
    {
        float                   clearColor[]         = {0, 0, 0, 1};
        auto                    graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11RenderTargetView* rtv                  = graphicsAdapterD3D11->GetMainRTV();
        ID3D11DepthStencilView* dsv                  = graphicsAdapterD3D11->GetDepthStencil();
        graphicsAdapterD3D11->GetDeviceContext()->ClearRenderTargetView(rtv, clearColor);
        graphicsAdapterD3D11->GetDeviceContext()->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
    }
} // namespace pge