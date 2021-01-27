#ifndef PGE_GRAPHICS_GFX_BUFFER_H
#define PGE_GRAPHICS_GFX_BUFFER_H

#include <memory>

namespace pge
{
    class gfx_GraphicsAdapter;

    enum class gfx_BufferUsage
    {
        STATIC,
        DYNAMIC
    };

    class gfx_ConstantBuffer {
        class gfx_ConstantBufferImpl;
        std::unique_ptr<gfx_ConstantBufferImpl> m_impl;

    public:
        gfx_ConstantBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage);
        ~gfx_ConstantBuffer();

        void Update(const void* data, size_t size);
        void BindVS(unsigned slot) const;
        void BindPS(unsigned slot) const;
    };


    class gfx_IndexBuffer {
        class gfx_IndexBufferImpl;
        std::unique_ptr<gfx_IndexBufferImpl> m_impl;

    public:
        gfx_IndexBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage);
        gfx_IndexBuffer(gfx_IndexBuffer&& other) noexcept;
        ~gfx_IndexBuffer();

        void Update(const void* data, size_t size, size_t offset);
        void Bind(size_t offset) const;
    };


    class gfx_VertexBuffer {
        class gfx_VertexBufferImpl;
        std::unique_ptr<gfx_VertexBufferImpl> m_impl;

    public:
        gfx_VertexBuffer(gfx_GraphicsAdapter* graphicsAdapter, const void* data, size_t size, gfx_BufferUsage usage);
        gfx_VertexBuffer(gfx_VertexBuffer&& other) noexcept;
        ~gfx_VertexBuffer();

        void Update(const void* data, size_t size, size_t offset);
        void Bind(unsigned slot, size_t vertexStride, size_t offset) const;
    };
}

#endif