#ifndef PGE_RESOURCE_RES_TEXTURE2D_H
#define PGE_RESOURCE_RES_TEXTURE2D_H

#include <gfx_texture.h>
#include <memory.h>
#include <unordered_map>
#include <string>

namespace pge
{
    class res_Texture2D {
        int                            m_width;
        int                            m_height;
        std::unique_ptr<gfx_Texture2D> m_texture;

    public:
        res_Texture2D(gfx_GraphicsAdapter* graphicsAdapter, const char* path);
        int            GetWidth() const;
        int            GetHeight() const;
        gfx_Texture2D* GetTexture() const;
    };

    class res_Texture2DCache {
        gfx_GraphicsAdapter*                           m_graphicsAdapter;
        std::unordered_map<std::string, res_Texture2D> m_textureMap;

    public:
        explicit res_Texture2DCache(gfx_GraphicsAdapter* graphicsAdapter);
        res_Texture2D* Load(const char* path);
    };
} // namespace pge

#endif
