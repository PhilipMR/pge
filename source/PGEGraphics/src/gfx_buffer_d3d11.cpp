#include "../include/gfx_buffer.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <core_assert.h>
#include <comdef.h>

namespace pge
{
    static D3D11_USAGE
    GetBufferUsageD3D11(gfx_BufferUsage usage)
    {
        switch (usage) {
            case gfx_BufferUsage::STATIC: return D3D11_USAGE_DEFAULT;
            case gfx_BufferUsage::DYNAMIC: return D3D11_USAGE_DYNAMIC;
            default: core_CrashAndBurn("Encountered unmapped BufferUsage->D3D11_USAGE.");
        }
        return D3D11_USAGE_DEFAULT;
    }

    static ID3D11Buffer*
    CreateBufferD3D11(ID3D11Device* device, const void* data, size_t size, gfx_BufferUsage usage, UINT bindFlags)
    {
        D3D11_BUFFER_DESC bufferDesc = {0};
        bufferDesc.BindFlags         = bindFlags;
        bufferDesc.ByteWidth         = size;
        bufferDesc.CPUAccessFlags    = (usage == gfx_BufferUsage::DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        bufferDesc.Usage             = GetBufferUsageD3D11(usage);

        D3D11_SUBRESOURCE_DATA bufferData = {0};
        bufferData.pSysMem                = data;

        D3D11_SUBRESOURCE_DATA* bufferDataPtr = nullptr;
        if (data != nullptr)
            bufferDataPtr = &bufferData;

        ID3D11Buffer* buffer;
        HRESULT       result = device->CreateBuffer(&bufferDesc, bufferDataPtr, &buffer);
        core_AssertWithReason(SUCCEEDED(result), _com_error(result).ErrorMessage());
        return buffer;
    }

    static void
    UpdateBufferD3D11(ID3D11DeviceContext* context, ID3D11Buffer* buffer, const void* data, size_t size, size_t offset, gfx_BufferUsage usage)
    {
        switch (usage) {
            case gfx_BufferUsage::STATIC: {
                D3D11_BOX dstBox;
                dstBox.left          = offset;
                dstBox.right         = size;
                dstBox.top           = 0;
                dstBox.bottom        = 1;
                dstBox.front         = 0;
                dstBox.back          = 1;
                D3D11_BOX* dstBoxPtr = &dstBox;
                context->UpdateSubresource(buffer, 0, dstBoxPtr, data, size, 0);
            } break;
            case gfx_BufferUsage::DYNAMIC: {
                D3D11_MAPPED_SUBRESOURCE subData;
                HRESULT                  result = context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subData);
                core_AssertWithReason(SUCCEEDED(result), "Failed to map to dynamic buffer.");
                memcpy((void*)((char*)subData.pData + offset), data, size);
                context->Unmap(buffer, 0);
            } break;
            default: {
                core_CrashAndBurn("Unhandled gfx_BufferUsage case");
            } break;
        }
    }


    // ------------------------------------------------------------
    // gfx_ConstantBuffer
    // ------------------------------------------------------------
    struct gfx_ConstantBuffer::gfx_ConstantBufferImpl {
        ID3D11DeviceContext* m_deviceContext;
        gfx_BufferUsage      m_usage;
        ID3D11Buffer*        m_buffer;
        size_t               m_size;
    };

    gfx_ConstantBuffer::gfx_ConstantBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage)
        : m_impl(new gfx_ConstantBufferImpl)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        m_impl->m_deviceContext   = graphicsAdapterD3D11->GetDeviceContext();
        m_impl->m_usage           = usage;
        m_impl->m_size            = size;
        m_impl->m_buffer          = CreateBufferD3D11(graphicsAdapterD3D11->GetDevice(), data, size, usage, D3D11_BIND_CONSTANT_BUFFER);
    }

    gfx_ConstantBuffer::~gfx_ConstantBuffer()
    {
        m_impl->m_buffer->Release();
    }

    void
    gfx_ConstantBuffer::Update(const void* data, size_t size)
    {
        core_Assert(size == m_impl->m_size);
        UpdateBufferD3D11(m_impl->m_deviceContext, m_impl->m_buffer, data, size, 0, m_impl->m_usage);
    }

    void
    gfx_ConstantBuffer::BindVS(unsigned slot) const
    {
        m_impl->m_deviceContext->VSSetConstantBuffers(slot, 1, &m_impl->m_buffer);
    }

    void gfx_ConstantBuffer::BindGS(unsigned slot) const
    {
        m_impl->m_deviceContext->GSSetConstantBuffers(slot, 1, &m_impl->m_buffer);
    }

    void
    gfx_ConstantBuffer::BindPS(unsigned slot) const
    {
        m_impl->m_deviceContext->PSSetConstantBuffers(slot, 1, &m_impl->m_buffer);
    }



    // ------------------------------------------------------------
    // gfx_IndexBuffer
    // ------------------------------------------------------------
    struct gfx_IndexBuffer::gfx_IndexBufferImpl {
        ID3D11DeviceContext* m_deviceContext;
        gfx_BufferUsage      m_usage;
        ID3D11Buffer*        m_buffer;
    };

    gfx_IndexBuffer::gfx_IndexBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage)
        : m_impl(new gfx_IndexBufferImpl)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        m_impl->m_deviceContext                        = graphicsAdapterD3D11->GetDeviceContext();
        m_impl->m_usage                                = usage;
        m_impl->m_buffer = CreateBufferD3D11(graphicsAdapterD3D11->GetDevice(), data, size, usage, D3D11_BIND_INDEX_BUFFER);
    }

    gfx_IndexBuffer::gfx_IndexBuffer(gfx_IndexBuffer&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {}

    gfx_IndexBuffer::~gfx_IndexBuffer()
    {
        m_impl->m_buffer->Release();
    }

    void
    gfx_IndexBuffer::Update(const void* data, size_t size, size_t offset)
    {
        UpdateBufferD3D11(m_impl->m_deviceContext, m_impl->m_buffer, data, size, offset, m_impl->m_usage);
    }

    void
    gfx_IndexBuffer::Bind(size_t offset) const
    {
        m_impl->m_deviceContext->IASetIndexBuffer(m_impl->m_buffer, DXGI_FORMAT_R32_UINT, offset);
    }


    // ------------------------------------------------------------
    // gfx_VertexBuffer
    // ------------------------------------------------------------
    struct gfx_VertexBuffer::gfx_VertexBufferImpl {
        ID3D11DeviceContext* m_deviceContext;
        gfx_BufferUsage      m_usage;
        ID3D11Buffer*        m_buffer;
    };

    gfx_VertexBuffer::gfx_VertexBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage)
        : m_impl(new gfx_VertexBufferImpl)
    {
        auto graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        m_impl->m_deviceContext                        = graphicsAdapterD3D11->GetDeviceContext();
        m_impl->m_usage                                = usage;
        m_impl->m_buffer = CreateBufferD3D11(graphicsAdapterD3D11->GetDevice(), data, size, usage, D3D11_BIND_VERTEX_BUFFER);
    }

    gfx_VertexBuffer::gfx_VertexBuffer(gfx_VertexBuffer&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {}

    gfx_VertexBuffer::~gfx_VertexBuffer()
    {
        m_impl->m_buffer->Release();
    }

    void
    gfx_VertexBuffer::Update(const void* data, size_t size, size_t offset)
    {
        UpdateBufferD3D11(m_impl->m_deviceContext, m_impl->m_buffer, data, size, offset, m_impl->m_usage);
    }

    void
    gfx_VertexBuffer::Bind(unsigned slot, size_t vertexStride, size_t offset) const
    {
        m_impl->m_deviceContext->IASetVertexBuffers(slot,
                                                    1,
                                                    &m_impl->m_buffer,
                                                    reinterpret_cast<UINT*>(&vertexStride),
                                                    reinterpret_cast<UINT*>(&offset));
    }
} // namespace pge