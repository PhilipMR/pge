#include "../include/res_texture2d.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <core_assert.h>

namespace pge
{
    res_Texture2D::res_Texture2D(gfx_GraphicsAdapter* graphicsAdapter, const char* path)
    {
        FILE* file = nullptr;
        fopen_s(&file, path, "rb");
        assert(file != nullptr);
        const int      desiredChannels = 4;
        unsigned char* textureData     = stbi_load_from_file(file, &m_width, &m_height, nullptr, desiredChannels);
        core_Assert(textureData != nullptr);
        fclose(file);

        m_texture = std::make_unique<gfx_Texture2D>(graphicsAdapter, gfx_PixelFormat::R8G8B8A8_UNORM, m_width, m_height, textureData);
        free(textureData);
    }

    int
    res_Texture2D::GetWidth() const
    {
        return m_width;
    }

    int
    res_Texture2D::GetHeight() const
    {
        return m_height;
    }

    gfx_Texture2D*
    res_Texture2D::GetTexture() const
    {
        return m_texture.get();
    }


    // ---------------------------------
    // res_Texture2DCache
    // ---------------------------------
    res_Texture2DCache::res_Texture2DCache(gfx_GraphicsAdapter* graphicsAdapter)
        : m_graphicsAdapter(graphicsAdapter)
    {}

    res_Texture2D*
    res_Texture2DCache::Load(const char* path)
    {
        auto it = m_textureMap.find(path);
        if (it == m_textureMap.end()) {
            m_textureMap.emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple(m_graphicsAdapter, path));
        }
        return &m_textureMap.at(path);
    }
} // namespace pge