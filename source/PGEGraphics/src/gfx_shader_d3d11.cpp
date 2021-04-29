#include "../include/gfx_shader.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <core_assert.h>
#include <d3dcompiler.h>

namespace pge
{
    // ------------------------------------------------------------
    // gfx_VertexShader
    // ------------------------------------------------------------
    struct gfx_VertexShader::gfx_VertexShaderImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11VertexShader*  m_shader;
    };

    gfx_VertexShader::gfx_VertexShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize)
        : m_impl(std::make_unique<gfx_VertexShaderImpl>())
    {
        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob *shaderBlob, *errorBlob;
        HRESULT   result = D3DCompile(source, sourceSize, nullptr, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        core_AssertWithReason(SUCCEEDED(result), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();

        ID3D11VertexShader* shader;
        result = device->CreateVertexShader(reinterpret_cast<char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize(), nullptr, &shader);
        core_Assert(SUCCEEDED(result));
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

    void gfx_VertexShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        graphicsAdapterD3D11->GetDeviceContext()->VSSetShader(nullptr, nullptr, 0);
    }

    // ------------------------------------------------------------
    // gfx_PixelShader
    // ------------------------------------------------------------
    struct gfx_PixelShader::gfx_PixelShaderImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11PixelShader*   m_shader;
    };

    gfx_PixelShader::gfx_PixelShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize)
        : m_impl(std::make_unique<gfx_PixelShaderImpl>())
    {
        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob *shaderBlob, *errorBlob;
        HRESULT   result = D3DCompile(source, sourceSize, nullptr, nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        core_AssertWithReason(SUCCEEDED(result), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();
        m_impl->m_deviceContext            = graphicsAdapterD3D11->GetDeviceContext();

        result = device->CreatePixelShader(reinterpret_cast<char*>(shaderBlob->GetBufferPointer()),
                                           shaderBlob->GetBufferSize(),
                                           nullptr,
                                           &m_impl->m_shader);
        core_Assert(SUCCEEDED(result));
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

    void gfx_PixelShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        graphicsAdapterD3D11->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
    }


    // ------------------------------------------------------------
    // gfx_GeometryShader
    // ------------------------------------------------------------
    struct gfx_GeometryShader::gfx_GeometryShaderImpl {
        ID3D11DeviceContext*  m_deviceContext;
        ID3D11GeometryShader* m_shader;
    };

    gfx_GeometryShader::gfx_GeometryShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize)
        : m_impl(std::make_unique<gfx_GeometryShaderImpl>())
    {
        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob *shaderBlob, *errorBlob;
        HRESULT   result = D3DCompile(source, sourceSize, nullptr, nullptr, nullptr, "GSMain", "gs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        core_AssertWithReason(SUCCEEDED(result), reinterpret_cast<char*>(errorBlob->GetBufferPointer()));

        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();
        m_impl->m_deviceContext            = graphicsAdapterD3D11->GetDeviceContext();

        result = device->CreateGeometryShader(reinterpret_cast<char*>(shaderBlob->GetBufferPointer()),
                                           shaderBlob->GetBufferSize(),
                                           nullptr,
                                           &m_impl->m_shader);
        core_Assert(SUCCEEDED(result));
        shaderBlob->Release();
    }

    gfx_GeometryShader::~gfx_GeometryShader()
    {
        m_impl->m_shader->Release();
    }

    void
    gfx_GeometryShader::Bind() const
    {
        m_impl->m_deviceContext->GSSetShader(m_impl->m_shader, nullptr, 0);
    }

    void gfx_GeometryShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        graphicsAdapterD3D11->GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);
    }
} // namespace pge