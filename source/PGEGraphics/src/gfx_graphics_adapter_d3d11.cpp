#include "../include/gfx_graphics_adapter_d3d11.h"
#include <diag_assert.h>
#include <comdef.h>

namespace pge
{
    struct gfx_GraphicsAdapter::gfx_GraphicsAdapterImpl {
        IDXGISwapChain*         m_swapChain;
        ID3D11Device*           m_device;
        ID3D11DeviceContext*    m_deviceContext;
        D3D_FEATURE_LEVEL       m_featureLevel;
        ID3D11RenderTargetView* m_mainRtv;
        ID3D11DepthStencilView* m_mainDsv;
        ID3D11RasterizerState*  m_rasterizerState;
    };

    static const DXGI_SAMPLE_DESC SWAPCHAIN_SAMPLE_DESC{8, 0};

    void
    CreateBackBuffers(IDXGISwapChain*          swapChain,
                      ID3D11Device*            device,
                      unsigned                 width,
                      unsigned                 height,
                      ID3D11RenderTargetView** rtvOut,
                      ID3D11DepthStencilView** dsvOut)
    {
        // Create back buffer RTV.
        ID3D11Texture2D* backBuffer;
        HRESULT result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        diag_AssertWithReason(SUCCEEDED(result), _com_error(result).ErrorMessage());

        result = device->CreateRenderTargetView(backBuffer, nullptr, rtvOut);
        backBuffer->Release();
        diag_AssertWithReason(SUCCEEDED(result), _com_error(result).ErrorMessage());

        // Create depth stencil view.
        D3D11_TEXTURE2D_DESC depthTextureDesc = {};
        depthTextureDesc.Width                = width;
        depthTextureDesc.Height               = height;
        depthTextureDesc.MipLevels            = 1;
        depthTextureDesc.ArraySize            = 1;
        depthTextureDesc.SampleDesc           = SWAPCHAIN_SAMPLE_DESC;
        depthTextureDesc.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthTextureDesc.BindFlags            = D3D11_BIND_DEPTH_STENCIL;

        ID3D11Texture2D* depthTexture;
        result = device->CreateTexture2D(&depthTextureDesc, nullptr, &depthTexture);
        diag_Assert(SUCCEEDED(result));

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format                        = depthTextureDesc.Format;
        depthStencilDesc.ViewDimension                 = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        result                                         = device->CreateDepthStencilView(depthTexture, &depthStencilDesc, dsvOut);
        depthTexture->Release();
        diag_Assert(SUCCEEDED(result));
    }


    gfx_GraphicsAdapter::gfx_GraphicsAdapter()
        : m_impl(new gfx_GraphicsAdapterImpl)
    {}

    gfx_GraphicsAdapter::~gfx_GraphicsAdapter() = default;

    gfx_GraphicsAdapterD3D11::gfx_GraphicsAdapterD3D11(HWND hwnd, unsigned width, unsigned height)
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        swapChainDesc.BufferDesc.Width                   = width;
        swapChainDesc.BufferDesc.Height                  = height;
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 1;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
        swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SampleDesc                         = SWAPCHAIN_SAMPLE_DESC;
        swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount                        = 1;
        swapChainDesc.OutputWindow                       = hwnd;
        swapChainDesc.Windowed                           = true;
        swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags                              = 0;

        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

        UINT deviceFlags = 0;
#ifdef _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr,
                                                       D3D_DRIVER_TYPE_HARDWARE,
                                                       nullptr,
                                                       deviceFlags,
                                                       featureLevels,
                                                       ARRAYSIZE(featureLevels),
                                                       D3D11_SDK_VERSION,
                                                       &swapChainDesc,
                                                       &m_impl->m_swapChain,
                                                       &m_impl->m_device,
                                                       &m_impl->m_featureLevel,
                                                       &m_impl->m_deviceContext);
        diag_AssertWithReason(SUCCEEDED(result), _com_error(result).ErrorMessage());

        CreateBackBuffers(m_impl->m_swapChain, m_impl->m_device, width, height, &m_impl->m_mainRtv, &m_impl->m_mainDsv);
        m_impl->m_deviceContext->OMSetRenderTargets(1, &m_impl->m_mainRtv, m_impl->m_mainDsv);

        D3D11_VIEWPORT viewport = {0};
        viewport.Width          = width;
        viewport.Height         = height;
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        m_impl->m_deviceContext->RSSetViewports(1, &viewport);


        // Create the rasterizer state.
        D3D11_RASTERIZER_DESC rasterizerDesc;
        rasterizerDesc.FillMode              = D3D11_FILL_SOLID; // D3D11_FILL_WIREFRAME;
        rasterizerDesc.CullMode              = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = true;
        rasterizerDesc.DepthBias             = 0;
        rasterizerDesc.DepthBiasClamp        = 0;
        rasterizerDesc.SlopeScaledDepthBias  = 0;
        rasterizerDesc.DepthClipEnable       = true;
        rasterizerDesc.ScissorEnable         = false;
        rasterizerDesc.MultisampleEnable     = true;
        rasterizerDesc.AntialiasedLineEnable = false;

        result = m_impl->m_device->CreateRasterizerState(&rasterizerDesc, &m_impl->m_rasterizerState);
        diag_Assert(SUCCEEDED(result));
        m_impl->m_deviceContext->RSSetState(m_impl->m_rasterizerState);
    }

    gfx_GraphicsAdapterD3D11::~gfx_GraphicsAdapterD3D11()
    {
        m_impl->m_rasterizerState->Release();
        m_impl->m_mainDsv->Release();
        m_impl->m_mainRtv->Release();
        m_impl->m_deviceContext->Release();
        m_impl->m_device->Release();
        m_impl->m_swapChain->Release();
    }

    void
    gfx_GraphicsAdapterD3D11::ResizeBackBuffer(unsigned width, unsigned height)
    {
        m_impl->m_mainDsv->Release();
        m_impl->m_mainRtv->Release();
        m_impl->m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

        CreateBackBuffers(m_impl->m_swapChain, m_impl->m_device, width, height, &m_impl->m_mainRtv, &m_impl->m_mainDsv);
        m_impl->m_deviceContext->OMSetRenderTargets(1, &m_impl->m_mainRtv, m_impl->m_mainDsv);
    }

    ID3D11Device*
    gfx_GraphicsAdapterD3D11::GetDevice()
    {
        return m_impl->m_device;
    }

    ID3D11DeviceContext*
    gfx_GraphicsAdapterD3D11::GetDeviceContext()
    {
        return m_impl->m_deviceContext;
    }

    ID3D11RenderTargetView*
    gfx_GraphicsAdapterD3D11::GetMainRTV()
    {
        return m_impl->m_mainRtv;
    }

    ID3D11DepthStencilView*
    gfx_GraphicsAdapterD3D11::GetDepthStencil()
    {
        return m_impl->m_mainDsv;
    }

    IDXGISwapChain*
    gfx_GraphicsAdapterD3D11::GetSwapChain()
    {
        return m_impl->m_swapChain;
    }
} // namespace pge