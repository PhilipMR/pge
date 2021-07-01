#ifndef PGE_RESOURCE_RES_EFFECT_H
#define PGE_RESOURCE_RES_EFFECT_H

#include <gfx_shader.h>
#include <core_assert.h>
#include <memory>
#include <unordered_map>

namespace pge
{
    enum class res_EffectPropertyType
    {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        TEXTURE2D
    };

    inline size_t
    res_EffectPropertyType_GetSize(res_EffectPropertyType type)
    {
        switch (type) {
            case res_EffectPropertyType::FLOAT: return sizeof(float) * 1;
            case res_EffectPropertyType::FLOAT2: return sizeof(float) * 2;
            case res_EffectPropertyType::FLOAT3: return sizeof(float) * 3;
            case res_EffectPropertyType::FLOAT4: return sizeof(float) * 4;
            case res_EffectPropertyType::TEXTURE2D: return 0; // Not stored in constant buffer.
            default: core_CrashAndBurn("No mapping for res_EffectPropertyType.");
        }
        return 0;
    }

    inline res_EffectPropertyType
    res_EffectPropertyType_FromString(const char* string)
    {
        if (strcmp(string, "float") == 0)
            return res_EffectPropertyType::FLOAT;
        else if (strcmp(string, "float2") == 0)
            return res_EffectPropertyType::FLOAT2;
        else if (strcmp(string, "float3") == 0)
            return res_EffectPropertyType::FLOAT3;
        else if (strcmp(string, "float4") == 0)
            return res_EffectPropertyType::FLOAT4;
        else if (strcmp(string, "Texture2D") == 0)
            return res_EffectPropertyType::TEXTURE2D;
        core_CrashAndBurn("Unhandled res_EffectPropertyType string.");
        return res_EffectPropertyType::FLOAT;
    }

    class res_EffectProperty {
        char                   m_name[64];
        res_EffectPropertyType m_type;

    public:
        res_EffectProperty();
        res_EffectProperty(const char* name, res_EffectPropertyType type);

        const char*            Name() const;
        res_EffectPropertyType Type() const;
        size_t                 Size() const;
    };

    class res_Effect {
        static const size_t               MaxProperties = 10;
        std::unique_ptr<gfx_VertexShader> m_vertexShader;
        std::unique_ptr<gfx_PixelShader>  m_pixelShader;
        size_t                            m_numProperties;
        res_EffectProperty                m_properties[MaxProperties];

    public:
        res_Effect(gfx_GraphicsAdapter* graphicsAdapter, const char* path);
        res_Effect(gfx_GraphicsAdapter*      graphicsAdapter,
                   const char*               vsSource,
                   const char*               psSource,
                   const res_EffectProperty* properties,
                   size_t                    numProperties);

        void   Bind() const;
        size_t GetPropertyOffset(const char* name) const;
        size_t GetPropertySize(const char* name) const;
        size_t GetPropertiesCBSize() const;
        size_t GetTextureSlot(const char* name) const;
    };

    class res_EffectCache {
        gfx_GraphicsAdapter*                        m_graphicsAdapter;
        std::unordered_map<std::string, res_Effect> m_effectMap;

    public:
        explicit res_EffectCache(gfx_GraphicsAdapter* graphicsAdapter);
        res_Effect* Load(const char* path);
    };
} // namespace pge

#endif