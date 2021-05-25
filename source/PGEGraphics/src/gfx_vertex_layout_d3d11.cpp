#include "../include/gfx_vertex_layout.h"
#include "../include/gfx_graphics_adapter_d3d11.h"
#include <core_assert.h>
#include <d3dcompiler.h>
#include <comdef.h>
#include <vector>
#include <sstream>

namespace pge
{
    // -----------------------------------------------
    // gfx_VertexAttribute
    // -----------------------------------------------
    gfx_VertexAttribute::gfx_VertexAttribute()
        : m_name("")
        , m_type(gfx_VertexAttributeType::UNASSIGNED)
    {}

    gfx_VertexAttribute::gfx_VertexAttribute(const char* name, gfx_VertexAttributeType type)
        : m_type(type)
    {
        strcpy_s(m_name, name);
    }

    gfx_VertexAttribute&
    gfx_VertexAttribute::operator=(const gfx_VertexAttribute& rhs)
    {
        if (&rhs == this)
            return *this;
        strcpy_s(m_name, rhs.m_name);
        m_type = rhs.m_type;
        return *this;
    }

    const char*
    gfx_VertexAttribute::Name() const
    {
        return m_name;
    }

    gfx_VertexAttributeType
    gfx_VertexAttribute::Type() const
    {
        return m_type;
    }


    // -----------------------------------------------
    // gfx_VertexLayout
    // -----------------------------------------------
    static DXGI_FORMAT
    GetVertexAttributeTypeDXGI(gfx_VertexAttributeType type)
    {
        switch (type) {
            case gfx_VertexAttributeType::FLOAT: return DXGI_FORMAT_R32_FLOAT;
            case gfx_VertexAttributeType::FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
            case gfx_VertexAttributeType::FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case gfx_VertexAttributeType::FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case gfx_VertexAttributeType::INT4: return DXGI_FORMAT_R32G32B32A32_SINT;
            default: throw std::runtime_error("Unhandled gfx_VertexAttributeType");
        }
    }

    static const char*
    GetInputElementTypeHLSL(const D3D11_INPUT_ELEMENT_DESC& input)
    {
        switch (input.Format) {
            case DXGI_FORMAT_R32_FLOAT: return "float";
            case DXGI_FORMAT_R32G32_FLOAT: return "float2";
            case DXGI_FORMAT_R32G32B32_FLOAT: return "float3";
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return "float4";
            case DXGI_FORMAT_R32G32B32A32_SINT: return "int4";
            default: throw std::runtime_error("Unhandled DXGI format");
        }
    }

    static ID3DBlob*
    CreateDummyShaderBlobD3D11(D3D11_INPUT_ELEMENT_DESC* inputs, size_t inputCount)
    {
        std::stringstream ss;
        ss << "struct vertex_data{\n";
        char attribName = 'a';
        for (size_t i = 0; i < inputCount; ++i) {
            ss << GetInputElementTypeHLSL(inputs[i]) << " " << attribName << " : " << inputs[i].SemanticName << ";\n";
            attribName++;
        }
        ss << "\n};\nfloat4 VSMain(vertex_data Vertex) : SV_POSITION { return float4(0,0,0,0); }";

        std::string source = ss.str();

        UINT compileFlags = 0;
#ifdef _DEBUG
        compileFlags |= D3DCOMPILE_DEBUG;
#endif
        ID3DBlob* shaderBlob;
        ID3DBlob* errorBlob;
        HRESULT   result
            = D3DCompile(source.c_str(), source.size(), nullptr, nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);
        core_AssertWithReason(SUCCEEDED(result), reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));

        return shaderBlob;
    }

    struct gfx_VertexLayout::gfx_VertexLayoutImpl {
        ID3D11DeviceContext* m_deviceContext;
        ID3D11InputLayout*   m_layout;
    };

    gfx_VertexLayout::gfx_VertexLayout(gfx_GraphicsAdapter* graphicsAdapter, const gfx_VertexAttribute* attributes, size_t numAttributes)
        : m_impl(new gfx_VertexLayoutImpl)
    {
        auto          graphicsAdapterD3D11 = reinterpret_cast<gfx_GraphicsAdapterD3D11*>(graphicsAdapter);
        ID3D11Device* device               = graphicsAdapterD3D11->GetDevice();
        m_impl->m_deviceContext            = graphicsAdapterD3D11->GetDeviceContext();

        std::vector<D3D11_INPUT_ELEMENT_DESC> inputs(numAttributes);
        for (size_t i = 0; i < inputs.size(); ++i) {
            D3D11_INPUT_ELEMENT_DESC input;
            input.SemanticName         = attributes[i].Name();
            input.SemanticIndex        = 0;
            input.Format               = GetVertexAttributeTypeDXGI(attributes[i].Type());
            input.InputSlot            = 0;
            input.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
            input.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
            input.InstanceDataStepRate = 0;
            inputs[i]                  = input;
        }
        ID3DBlob* shaderBlob = CreateDummyShaderBlobD3D11(&inputs[0], inputs.size());
        HRESULT   result
            = device->CreateInputLayout(&inputs[0], inputs.size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &m_impl->m_layout);
        core_AssertWithReason(SUCCEEDED(result), _com_error(result).ErrorMessage());
        shaderBlob->Release();
    }

    gfx_VertexLayout::gfx_VertexLayout(gfx_VertexLayout&& other) noexcept
        : m_impl(std::move(other.m_impl))
    {}

    gfx_VertexLayout::~gfx_VertexLayout()
    {
        m_impl->m_layout->Release();
    }

    void
    gfx_VertexLayout::Bind() const
    {
        m_impl->m_deviceContext->IASetInputLayout(m_impl->m_layout);
    }

} // namespace pge