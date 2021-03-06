#ifndef PGE_GRAPHICS_GFX_SHADER_H
#define PGE_GRAPHICS_GFX_SHADER_H

#include <memory>

namespace pge
{
    class gfx_GraphicsAdapter;

    class gfx_VertexShader {
        class gfx_VertexShaderImpl;
        std::unique_ptr<gfx_VertexShaderImpl> m_impl;

    public:
        gfx_VertexShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize);
        ~gfx_VertexShader();
        void Bind() const;
    };
    void gfx_VertexShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter);

    class gfx_PixelShader {
        class gfx_PixelShaderImpl;
        std::unique_ptr<gfx_PixelShaderImpl> m_impl;

    public:
        gfx_PixelShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize);
        ~gfx_PixelShader();
        void Bind() const;
    };
    void gfx_PixelShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter);

    class gfx_GeometryShader {
        class gfx_GeometryShaderImpl;
        std::unique_ptr<gfx_GeometryShaderImpl> m_impl;

    public:
        gfx_GeometryShader(gfx_GraphicsAdapter* graphicsAdapter, const char* source, size_t sourceSize);
        ~gfx_GeometryShader();
        void Bind() const;
    };
    void gfx_GeometryShader_Unbind(gfx_GraphicsAdapter* graphicsAdapter);

} // namespace pge

#endif