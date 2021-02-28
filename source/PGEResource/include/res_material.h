#ifndef PGE_RESOURCE_RES_MATERIAL_H
#define PGE_RESOURCE_RES_MATERIAL_H

#include "res_effect.h"
#include "res_texture2d.h"
#include <gfx_buffer.h>
#include <gfx_texture.h>
#include <gfx_sampler.h>
#include <unordered_map>

namespace pge
{
    class res_Material {
        std::string                         m_path;
        const res_Effect*                   m_effect;
        std::unique_ptr<gfx_ConstantBuffer> m_cbProperties;
        std::unique_ptr<char[]>             m_cbData;
        gfx_Sampler                         m_sampler;

        static const unsigned MaxTextures = 3;
        gfx_Texture2D*        m_textures[MaxTextures];

    public:
        res_Material(gfx_GraphicsAdapter* graphicsAdapter, const res_Effect* effect);
        res_Material(gfx_GraphicsAdapter* graphicsAdapter, res_EffectCache* effectCache, res_Texture2DCache* texCache, const char* path);

        void Bind() const;

        const res_Effect* GetEffect() const;
        const std::string GetPath() const;

        template <typename T>
        void
        SetProperty(size_t offset, const T& value)
        {
            diag_Assert(m_effect->PropertiesCBSize() > 0);
            diag_Assert(offset + sizeof(value) <= m_effect->PropertiesCBSize());
            memcpy(m_cbData.get() + offset, &value, sizeof(T));
            m_cbProperties->Update(m_cbData.get(), m_effect->PropertiesCBSize());
        }

        template <typename T>
        void
        SetProperty(const char* name, const T& value)
        {
            static_assert(!std::is_same<gfx_Texture2D, T>(), "When passing resources, only pointers are allowed (i.e. gfx_Texture2D*).");
            diag_AssertWithReason(m_effect->GetPropertySize(name) == sizeof(T), "The property expected a different value type.");
            SetProperty(m_effect->GetPropertyOffset(name), value);
        }

        template <typename T = gfx_Texture2D*>
        void
        SetProperty(const char* name, gfx_Texture2D* texture)
        {
            unsigned slot    = m_effect->GetTextureSlot(name);
            m_textures[slot] = texture;
        }
    };

    class res_MaterialCache {
        gfx_GraphicsAdapter*                          m_graphicsAdapter;
        res_EffectCache*                              m_effectCache;
        res_Texture2DCache*                           m_texCache;
        std::unordered_map<std::string, res_Material> m_materialMap;

    public:
        res_MaterialCache(gfx_GraphicsAdapter* graphicsAdapter, res_EffectCache* effectCache, res_Texture2DCache* texCache);
        res_Material* Load(const char* path);
    };
} // namespace pge

#endif
