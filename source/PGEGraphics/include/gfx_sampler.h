#ifndef PGE_GRAPHICS_GFX_SAMPLER_H
#define PGE_GRAPHICS_GFX_SAMPLER_H

#include <memory>

namespace pge
{
    class gfx_GraphicsAdapter;
    class gfx_Sampler {
        class gfx_SamplerImpl;
        std::unique_ptr<gfx_SamplerImpl> m_impl;

    public:
        explicit gfx_Sampler(gfx_GraphicsAdapter* graphicsAdapter);
        ~gfx_Sampler();
        void Bind(unsigned slot) const;
    };
} // namespace pge

#endif