#include "../include/res_effect.h"
#include <fstream>
#include <string>

namespace pge
{
    // ---------------------------------
    // res_EffectProperty
    // ---------------------------------
    res_EffectProperty::res_EffectProperty() = default;

    res_EffectProperty::res_EffectProperty(const char* name, res_EffectPropertyType type)
        : m_type(type)
    {
        strcpy_s(this->m_name, name);
    }

    const char*
    res_EffectProperty::Name() const
    {
        return m_name;
    }

    res_EffectPropertyType
    res_EffectProperty::Type() const
    {
        return m_type;
    }

    size_t
    res_EffectProperty::Size() const
    {
        return res_EffectPropertyType_GetSize(m_type);
    }


    // ---------------------------------
    // res_Effect
    // ---------------------------------
    static void
    ReadEffectFile(const char* path, res_EffectProperty* propertiesOut, size_t* numPropertiesOut, std::string* vsSourceOut, std::string* psSourceOut)
    {
        *numPropertiesOut = 0;
        *vsSourceOut      = "";
        *psSourceOut      = "";

        enum class ReadMode
        {
            SCAN_STRUCTURE,
            PROPERTIES,
            VERTEX_SHADER,
            PIXEL_SHADER
        };
        ReadMode      readMode = ReadMode::SCAN_STRUCTURE;
        std::string   line;
        std::ifstream file(path);
        while (std::getline(file, line)) {
            switch (readMode) {
                case ReadMode::SCAN_STRUCTURE: {
                    char structure[32];
                    if (sscanf(line.c_str(), "%31s {", structure) != 1)
                        break;
                    if (strcmp(structure, "Properties") == 0) {
                        readMode = ReadMode::PROPERTIES;
                    } else if (strcmp(structure, "VertexShader") == 0) {
                        readMode = ReadMode::VERTEX_SHADER;
                    } else if (strcmp(structure, "PixelShader") == 0) {
                        readMode = ReadMode::PIXEL_SHADER;
                    } else {
                        diag_CrashAndBurn("Unhandled effect structure type.");
                    }
                } break;

                case ReadMode::PROPERTIES: {
                    if (line == "}") {
                        readMode = ReadMode::SCAN_STRUCTURE;
                    } else {
                        char type[32];
                        char id[32];
                        if (sscanf(line.c_str(), "%31s %31s", type, id) != 2)
                            break;
                        propertiesOut[(*numPropertiesOut)++] = res_EffectProperty(id, res_EffectPropertyType_FromString(type));
                    }
                } break;

                case ReadMode::VERTEX_SHADER: {
                    if (line == "}") {
                        readMode = ReadMode::SCAN_STRUCTURE;
                    } else {
                        *vsSourceOut += line; // BAD
                    }

                } break;

                case ReadMode::PIXEL_SHADER: {
                    if (line == "}") {
                        readMode = ReadMode::SCAN_STRUCTURE;
                    } else {
                        *psSourceOut += line; // BAD
                    }
                } break;

                default: diag_CrashAndBurn("Unmapped ReadMode.");
            }
        }
    }

    res_Effect::res_Effect(gfx_GraphicsAdapter* graphicsAdapter, const char* path)
    {
        std::string vsSource;
        std::string psSource;
        ReadEffectFile(path, &m_properties[0], &m_numProperties, &vsSource, &psSource);
        diag_Assert(m_numProperties < MaxProperties);
        m_vertexShader = std::make_unique<gfx_VertexShader>(graphicsAdapter, vsSource.c_str(), vsSource.size());
        m_pixelShader  = std::make_unique<gfx_PixelShader>(graphicsAdapter, psSource.c_str(), psSource.size());
    }

    res_Effect::res_Effect(gfx_GraphicsAdapter*      graphicsAdapter,
                           const char*               vsSource,
                           const char*               psSource,
                           const res_EffectProperty* properties,
                           size_t                    numProperties)
        : m_numProperties(numProperties)
    {
        m_vertexShader = std::make_unique<gfx_VertexShader>(graphicsAdapter, vsSource, strlen(vsSource));
        m_pixelShader  = std::make_unique<gfx_PixelShader>(graphicsAdapter, psSource, strlen(psSource));
        diag_Assert(numProperties < MaxProperties);
        for (size_t i = 0; i < numProperties; ++i)
            m_properties[i] = properties[i];
    }

    const gfx_VertexShader*
    res_Effect::VertexShader() const
    {
        return m_vertexShader.get();
    }

    const gfx_PixelShader*
    res_Effect::PixelShader() const
    {
        return m_pixelShader.get();
    }

    size_t
    res_Effect::GetPropertyOffset(const char* name) const
    {
        size_t offset = 0;
        for (size_t i = 0; i < m_numProperties; ++i) {
            if (strcmp(m_properties[i].Name(), name) == 0)
                break;
            offset += m_properties->Size();
        }
        diag_Assert(offset < PropertiesCBSize());
        return offset;
    }

    size_t
    res_Effect::GetPropertySize(const char* name) const
    {
        for (size_t i = 0; i < m_numProperties; ++i) {
            if (strcmp(m_properties[i].Name(), name) == 0) {
                return m_properties[i].Size();
            }
        }
        return 0;
    }

    size_t
    res_Effect::PropertiesCBSize() const
    {
        size_t totalSize = 0;
        for (size_t i = 0; i < m_numProperties; ++i)
            totalSize += m_properties[i].Size();
        return totalSize;
    }

    size_t
    res_Effect::GetTextureSlot(const char* name) const
    {
        size_t slot = 0;
        for (size_t i = 0; i < m_numProperties; ++i) {
            if (strcmp(m_properties[i].Name(), name) == 0)
                break;
            if (m_properties[i].Type() == res_EffectPropertyType::TEXTURE2D)
                ++slot;
        }
        return slot;
    }



    // ---------------------------------
    // res_EffectCache
    // ---------------------------------
    res_EffectCache::res_EffectCache(gfx_GraphicsAdapter* graphicsAdapter)
        : m_graphicsAdapter(graphicsAdapter)
    {}

    res_Effect*
    res_EffectCache::Load(const char* path)
    {
        auto it = m_effectMap.find(path);
        if (it == m_effectMap.end()) {
            m_effectMap.emplace(std::piecewise_construct,
                                  std::make_tuple(path),
                                  std::make_tuple(m_graphicsAdapter, path));
        }
        return &m_effectMap.at(path);
    }
}