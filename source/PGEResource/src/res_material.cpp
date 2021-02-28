#include "../include/res_material.h"
#include <diag_assert.h>
#include <fstream>
#include <string>

namespace pge
{
    // ---------------------------------
    // res_Material
    // ---------------------------------
    res_Material::res_Material(gfx_GraphicsAdapter* graphicsAdapter, const res_Effect* effect)
        : m_path("<from-memory>")
        , m_effect(effect)
        , m_sampler(graphicsAdapter)
    {
        if (effect->PropertiesCBSize() > 0)
            m_cbProperties = std::make_unique<gfx_ConstantBuffer>(graphicsAdapter, nullptr, effect->PropertiesCBSize(), gfx_BufferUsage::DYNAMIC);
        m_cbData = std::unique_ptr<char[]>(new char[effect->PropertiesCBSize()]);
        for (auto& m_texture : m_textures)
            m_texture = nullptr;
    }

    res_Material::res_Material(gfx_GraphicsAdapter* graphicsAdapter, res_EffectCache* effectCache, res_Texture2DCache* texCache, const char* path)
        : m_path(path)
        , m_sampler(graphicsAdapter)
    {
        std::string   line;
        std::ifstream file(path);
        std::getline(file, line);
        char effectPath[128];
        diag_Verify(sscanf(line.c_str(), "Effect = %127s", effectPath) == 1);

        m_effect = effectCache->Load(effectPath);
        diag_Assert(m_effect != nullptr);
        if (m_effect->PropertiesCBSize() > 0)
            m_cbProperties = std::make_unique<gfx_ConstantBuffer>(graphicsAdapter, nullptr, m_effect->PropertiesCBSize(), gfx_BufferUsage::DYNAMIC);
        m_cbData = std::unique_ptr<char[]>(new char[m_effect->PropertiesCBSize()]);
        for (auto& m_texture : m_textures)
            m_texture = nullptr;

        bool readingProps = false;
        while (std::getline(file, line)) {
            if (line == "Properties {") {
                readingProps = true;
            } else if (line == "}") {
                readingProps = false;
            } else if (readingProps) {
                char typeStr[32];
                char id[32];
                char valueStr[128];
                if (sscanf(line.c_str(), "%31s %31[^(] %127[^\n]", typeStr, id, valueStr) != 3)
                    continue;

                auto type = res_EffectPropertyType_FromString(typeStr);
                switch (type) {
                    case res_EffectPropertyType::FLOAT: {
                        float value = 0.0f;
                        sscanf(valueStr, "(%f)", &value);
                        SetProperty(id, value);
                    } break;
                    case res_EffectPropertyType::FLOAT2: {
                        float xy[2];
                        sscanf(valueStr, "(%f, %f)", &xy[0], &xy[1]);
                        SetProperty(id, xy);
                    } break;
                    case res_EffectPropertyType::FLOAT3: {
                        float xyz[3];
                        sscanf(valueStr, "(%f, %f, %f)", &xyz[0], &xyz[1], &xyz[2]);
                        SetProperty(id, xyz);
                    } break;
                    case res_EffectPropertyType::FLOAT4: {
                        float xyzw[4];
                        sscanf(valueStr, "(%f, %f, %f, %f)", &xyzw[0], &xyzw[1], &xyzw[2], &xyzw[3]);
                        SetProperty(id, xyzw);
                    } break;
                    case res_EffectPropertyType::TEXTURE2D: {
                        char texPath[128];
                        sscanf(valueStr, "(%127[^)])", texPath);
                        res_Texture2D* tex = texCache->Load(texPath);
                        SetProperty(id, tex->GetTexture());
                    } break;
                }
            }
        }
    }

    void
    res_Material::Bind() const
    {
        m_sampler.Bind(0);
        m_cbProperties->BindPS(0);
        m_effect->VertexShader()->Bind();
        m_effect->PixelShader()->Bind();

        size_t numTextures = 0;
        while (m_textures[numTextures] != nullptr)
            ++numTextures;
        for (size_t i = 0; m_textures[i] != nullptr; ++i)
            m_textures[i]->Bind(i);
    }

    const res_Effect*
    res_Material::GetEffect() const
    {
        return m_effect;
    }

    const std::string
    res_Material::GetPath() const
    {
        return m_path;
    }


    // ---------------------------------
    // res_MaterialCache
    // ---------------------------------
    res_MaterialCache::res_MaterialCache(gfx_GraphicsAdapter* graphicsAdapter, res_EffectCache* effectCache, res_Texture2DCache* texCache)
        : m_graphicsAdapter(graphicsAdapter)
        , m_effectCache(effectCache)
        , m_texCache(texCache)
    {}

    res_Material*
    res_MaterialCache::Load(const char* path)
    {
        auto it = m_materialMap.find(path);
        if (it == m_materialMap.end()) {
            m_materialMap.emplace(std::piecewise_construct,
                                  std::make_tuple(path),
                                  std::make_tuple(m_graphicsAdapter, m_effectCache, m_texCache, path));
        }
        return &m_materialMap.at(path);
    }
} // namespace pge