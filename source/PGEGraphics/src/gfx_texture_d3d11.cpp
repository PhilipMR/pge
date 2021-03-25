#include "../include/gfx_texture.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <diag_assert.h>

namespace pge
{
    static DXGI_FORMAT
    GetFormatDXGI(gfx_PixelFormat format)
    {
        switch (format) {
            case gfx_PixelFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
            default: diag_CrashAndBurn("No mapping for gfx_PixelFormat.");
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    static UINT
    GetFormatSizeBytes(gfx_PixelFormat format)
    {
        switch (format) {
            case gfx_PixelFormat::R8G8B8A8_UNORM: return 4;
            default: diag_CrashAndBurn("No mapping for gfx_PixelFormat.");
        }
        return 0;
    }

    struct gfx_Texture2D::gfx_Texture2DImpl {
        ID3D11DeviceContext*      m_deviceContext;
        ID3D11Texture2D*          m_texture;
        ID3D11ShaderResourceView* m_srv;
    };

    gfx_Texture2D::gfx_Texture2D(gfx_GraphicsAdapter* graphicsAdapter, gfx_PixelFormat format, unsigned width, unsigned height, void* data)
        : m_impl(new gfx_Texture2DImpl)
    {
        auto                 graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device*        device               = graphicsAdapterD3D11->GetDevice();
        ID3D11DeviceContext* deviceContext        = graphicsAdapterD3D11->GetDeviceContext();
        m_impl->m_deviceContext                   = deviceContext;

        D3D11_TEXTURE2D_DESC textureDesc;
        textureDesc.Width              = width;
        textureDesc.Height             = height;
        textureDesc.MipLevels          = 0;
        textureDesc.ArraySize          = 1;
        textureDesc.Format             = GetFormatDXGI(format);
        textureDesc.SampleDesc.Count   = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage              = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        textureDesc.CPUAccessFlags     = 0;
        textureDesc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        UINT bytesInRow   = GetFormatSizeBytes(format) * width;
        UINT bytesInTotal = height * bytesInRow;

        HRESULT result = device->CreateTexture2D(&textureDesc, nullptr, &m_impl->m_texture);
        diag_Assert(SUCCEEDED(result));

        D3D11_BOX destBox = {};
        destBox.right     = width;
        destBox.bottom    = height;
        destBox.back      = 1;
        deviceContext->UpdateSubresource(m_impl->m_texture, 0, &destBox, data, bytesInRow, bytesInTotal);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format                    = textureDesc.Format;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels       = UINT_MAX;
        srvDesc.Texture2D.MostDetailedMip = 0;

        result = device->CreateShaderResourceView(m_impl->m_texture, &srvDesc, &m_impl->m_srv);
        deviceContext->GenerateMips(m_impl->m_srv);
        diag_Assert(SUCCEEDED(result));
    }

    gfx_Texture2D::~gfx_Texture2D()
    {
        m_impl->m_srv->Release();
        m_impl->m_texture->Release();
    }

    void
    gfx_Texture2D::Bind(unsigned slot) const
    {
        m_impl->m_deviceContext->PSSetShaderResources(slot, 1, &m_impl->m_srv);
    }

    void* gfx_Texture2D::GetNativeTexture() const
    {
        return m_impl->m_srv;
    }

    void gfx_Texture2D_Unbind(gfx_GraphicsAdapter* graphicsAdapter, unsigned slot)
    {
        auto                    graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11ShaderResourceView* srvs[] = { nullptr };
        graphicsAdapterD3D11->GetDeviceContext()->PSSetShaderResources(slot, 1, srvs);
    }
} // namespace pge