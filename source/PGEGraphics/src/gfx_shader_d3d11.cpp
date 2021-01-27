#include "../include/gfx_shader.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <diag_assert.h>
#include <d3dcompiler.h>

namespace pge
{
    struct gfx_VertexShader::gfx_VertexShaderImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11VertexShader*  m_shader;
    };
    gfx_VertexShader::gfx_VertexShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize)
        : m_impl(new gfx_VertexShaderImpl)
    {
        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob *shaderBlob, *errorBlob;
        HRESULT   result = D3DCompile(source, sourceSize, nullptr, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        diag_AssertWithReason(SUCCEEDED(result), reinterpret_cast<char *>(errorBlob->GetBufferPointer()));

        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();

        ID3D11VertexShader* shader;
        result = device->CreateVertexShader(reinterpret_cast<char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize(), nullptr, &shader);
        diag_Assert(SUCCEEDED(result));
        shaderBlob->Release();

        m_impl->m_deviceContext = graphicsAdapterD3D11->GetDeviceContext();
        m_impl->m_shader        = shader;
    }
    gfx_VertexShader::~gfx_VertexShader()
    {
        m_impl->m_shader->Release();
    }
    void
    gfx_VertexShader::Bind() const
    {
        m_impl->m_deviceContext->VSSetShader(m_impl->m_shader, nullptr, 0);
    }

    struct gfx_PixelShader::gfx_PixelShaderImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11PixelShader*   m_shader;
    };
    gfx_PixelShader::gfx_PixelShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize)
        : m_impl(new gfx_PixelShaderImpl)
    {
        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob *shaderBlob, *errorBlob;
        HRESULT   result = D3DCompile(source, sourceSize, nullptr, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        diag_AssertWithReason(SUCCEEDED(result), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();
        m_impl->m_deviceContext            = graphicsAdapterD3D11->GetDeviceContext();

        result = device->CreatePixelShader(reinterpret_cast<char*>(shaderBlob->GetBufferPointer()),
                                           shaderBlob->GetBufferSize(),
                                           nullptr,
                                           &m_impl->m_shader);
        diag_Assert(SUCCEEDED(result));
        shaderBlob->Release();
    }
    gfx_PixelShader::~gfx_PixelShader()
    {
        m_impl->m_shader->Release();
    }
    void
    gfx_PixelShader::Bind() const
    {
        m_impl->m_deviceContext->PSSetShader(m_impl->m_shader, nullptr, 0);
    }

} // namespace pge