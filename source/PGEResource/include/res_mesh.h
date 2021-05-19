 #ifndef PGE_RESOURCE_RES_MESH_H
#define PGE_RESOURCE_RES_MESH_H

#include <core_assert.h>
#include <gfx_buffer.h>
#include <gfx_vertex_layout.h>
#include <fstream>
#include <unordered_map>
#include <string>
#include <math_vec2.h>
#include <math_vec3.h>
#include <math_aabb.h>

namespace pge
{
    enum class res_SerializedVertexAttribute
    {
        POSITION,
        NORMAL,
        TEXTURECOORD,
        COLOR,
        TOTAL_MESH_ATTRIBUTES
    };

    inline gfx_VertexAttribute
    res_SerializedVertexAttribute_GetVertexAttribute(res_SerializedVertexAttribute attribute)
    {
        switch (attribute) {
            case res_SerializedVertexAttribute::POSITION: return gfx_VertexAttribute("POSITION", gfx_VertexAttributeType::FLOAT3);
            case res_SerializedVertexAttribute::NORMAL: return gfx_VertexAttribute("NORMAL", gfx_VertexAttributeType::FLOAT3);
            case res_SerializedVertexAttribute::TEXTURECOORD: return gfx_VertexAttribute("TEXTURECOORD", gfx_VertexAttributeType::FLOAT2);
            case res_SerializedVertexAttribute::COLOR: return gfx_VertexAttribute("COLOR", gfx_VertexAttributeType::FLOAT3);
            default: core_CrashAndBurn("Unmapped res_SerializedVertexAttribute.");
        }
        return gfx_VertexAttribute("", gfx_VertexAttributeType::UNASSIGNED);
    }

    inline uint16_t
    res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute attribute)
    {
        return (1 << (uint16_t)attribute);
    }

    inline size_t
    res_SerializedVertexAttribute_GetVertexStride(uint16_t attributeFlags)
    {
        size_t stride = 0;
        for (int i = 0; i < static_cast<int>(res_SerializedVertexAttribute::TOTAL_MESH_ATTRIBUTES); ++i) {
            res_SerializedVertexAttribute sattrib    = static_cast<res_SerializedVertexAttribute>(i);
            gfx_VertexAttribute           attrib     = res_SerializedVertexAttribute_GetVertexAttribute(sattrib);
            uint16_t                      attribFlag = res_SerializedVertexAttribute_GetFlag(sattrib);
            if (attributeFlags & attribFlag) {
                stride += gfx_VertexAttributeType_GetSize(attrib.Type());
            }
        }
        return stride;
    }


    class res_SerializedMesh {
        std::string                 m_path;
        uint16_t                    m_version;
        uint16_t                    m_attributeFlags;
        uint32_t                    m_numVertices;
        uint32_t                    m_vertexDataSize;
        uint32_t                    m_numTriangles;
        std::unique_ptr<char[]>     m_vertexData;
        std::unique_ptr<unsigned[]> m_triangleData;

        static const uint16_t Version = 1;

    public:
        explicit res_SerializedMesh(const char* path);
        res_SerializedMesh(math_Vec3* positions,
                           math_Vec3* normals,
                           math_Vec2* texcoords,
                           math_Vec3* colors,
                           size_t     numVertices,
                           unsigned*  triangleData,
                           size_t     numTriangles);


        void Write(std::ostream& output) const;

        std::string     GetPath() const;
        uint16_t        GetVersion() const;
        uint16_t        GetAttributeFlags() const;
        uint32_t        GetNumVertices() const;
        uint32_t        GetVertexDataSize() const;
        uint32_t        GetNumTriangles() const;
        const char*     GetVertexData() const;
        const unsigned* GetTriangleData() const;
        size_t          GetVertexStride() const;
        math_AABB       GetAABB() const;
    };

    class res_Mesh {
        std::string      m_path;
        gfx_VertexBuffer m_vertexBuffer;
        gfx_IndexBuffer  m_indexBuffer;
        gfx_VertexLayout m_vertexLayout;
        size_t           m_vertexStride;
        size_t           m_numTriangles;
        math_AABB        m_aabb;

    public:
        res_Mesh(gfx_GraphicsAdapter*       graphicsAdapter,
                 const gfx_VertexAttribute* attributes,
                 size_t                     numAttributes,
                 const void*                vertexData,
                 size_t                     vertexDataSize,
                 const unsigned*            indexData,
                 size_t                     numIndices);

        res_Mesh(res_Mesh&& other) noexcept;
        res_Mesh(gfx_GraphicsAdapter* graphicsAdapter, const res_SerializedMesh& smesh);
        res_Mesh(gfx_GraphicsAdapter* graphicsAdapter, const char* path);

        void        Bind() const;
        size_t      GetNumTriangles() const;
        math_AABB   GetAABB() const;
        std::string GetPath() const;
    };

    class res_MeshCache {
        gfx_GraphicsAdapter*                      m_graphicsAdapter;
        std::unordered_map<std::string, res_Mesh> m_meshMap;

    public:
        explicit res_MeshCache(gfx_GraphicsAdapter* graphicsAdapter);
        res_Mesh* Load(const char* path);
    };
} // namespace pge

#endif