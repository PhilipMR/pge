#ifndef PGE_GRAPHICS_GFX_VERTEX_LAYOUT_H
#define PGE_GRAPHICS_GFX_VERTEX_LAYOUT_H

#include <memory>
#include <core_assert.h>

namespace pge
{
    enum class gfx_VertexAttributeType
    {
        UNASSIGNED,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4
    };

    constexpr size_t
    gfx_VertexAttributeType_GetSize(const gfx_VertexAttributeType& type)
    {
        switch (type) {
            case gfx_VertexAttributeType::FLOAT: return sizeof(float);
            case gfx_VertexAttributeType::FLOAT2: return sizeof(float) * 2;
            case gfx_VertexAttributeType::FLOAT3: return sizeof(float) * 3;
            case gfx_VertexAttributeType::FLOAT4: return sizeof(float) * 4;
            default: core_CrashAndBurn("No mapping for gfx_VertexAttributeType.");
        }
        return 0;
    }


    class gfx_VertexAttribute {
        char                    m_name[64];
        gfx_VertexAttributeType m_type;

    public:
        gfx_VertexAttribute();
        gfx_VertexAttribute(const char* name, gfx_VertexAttributeType type);
        gfx_VertexAttribute& operator=(const gfx_VertexAttribute& rhs);

        const char*             Name() const;
        gfx_VertexAttributeType Type() const;
    };


    class gfx_GraphicsAdapter;
    class gfx_VertexLayout {
        class gfx_VertexLayoutImpl;
        std::unique_ptr<gfx_VertexLayoutImpl> m_impl;

    public:
        gfx_VertexLayout(gfx_GraphicsAdapter* graphicsAdapter, const gfx_VertexAttribute* attributes, size_t numAttributes);
        gfx_VertexLayout(gfx_VertexLayout&& other) noexcept;
        ~gfx_VertexLayout();
        void Bind() const;
    };
} // namespace pge

#endif