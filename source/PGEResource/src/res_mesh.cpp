#include "../include/res_mesh.h"

namespace pge
{
    // ----------------------------------------------
    // res_SerializedMesh
    // ----------------------------------------------
    static unsigned
    GetTriangleDataSize(uint32_t numTriangles)
    {
        return numTriangles * 3 * sizeof(unsigned);
    }

    res_SerializedMesh::res_SerializedMesh(const char* path)
    {
        std::ifstream input(path, std::ios::binary);
        diag_Assert(input.is_open());
        input.read((char*)&m_version, sizeof(m_version));
        input.read((char*)&m_attributeFlags, sizeof(m_attributeFlags));
        input.read((char*)&m_numVertices, sizeof(m_numVertices));
        input.read((char*)&m_vertexDataSize, sizeof(m_vertexDataSize));
        input.read((char*)&m_numTriangles, sizeof(m_numTriangles));

        m_vertexData   = std::unique_ptr<char[]>(new char[m_vertexDataSize]);
        m_triangleData = std::unique_ptr<unsigned[]>(new unsigned[m_numTriangles * 3]);
        input.read(m_vertexData.get(), m_vertexDataSize);
        input.read((char*)&m_triangleData[0], GetTriangleDataSize(m_numTriangles));
        input.close();
    }

    void
    res_SerializedMesh::Write(std::ostream& output)
    {
        output.write((char*)&m_version, sizeof(m_version));
        output.write((char*)&m_attributeFlags, sizeof(m_attributeFlags));
        output.write((char*)&m_numVertices, sizeof(m_numVertices));
        output.write((char*)&m_vertexDataSize, sizeof(m_vertexDataSize));
        output.write((char*)&m_numTriangles, sizeof(m_numTriangles));
        output.write((char*)m_vertexData.get(), m_vertexDataSize);
        output.write((char*)m_triangleData.get(), GetTriangleDataSize(m_numTriangles));
    }

    uint16_t
    res_SerializedMesh::GetVersion() const
    {
        return m_version;
    }

    uint16_t
    res_SerializedMesh::GetAttributeFlags() const
    {
        return m_attributeFlags;
    }

    uint32_t
    res_SerializedMesh::GetNumVertices() const
    {
        return m_numVertices;
    }

    uint32_t
    res_SerializedMesh::GetVertexDataSize() const
    {
        return m_vertexDataSize;
    }

    uint32_t
    res_SerializedMesh::GetNumTriangles() const
    {
        return m_numTriangles;
    }

    const char*
    res_SerializedMesh::GetVertexData() const
    {
        return m_vertexData.get();
    }

    const unsigned*
    res_SerializedMesh::GetTriangleData() const
    {
        return m_triangleData.get();
    }

    size_t
    res_SerializedMesh::GetVertexStride() const
    {
        gfx_VertexAttribute attributes[(int)res_SerializedVertexAttribute::TOTAL_MESH_ATTRIBUTES];
        size_t              numAttributes = 0;
        uint16_t            attribFlags   = GetAttributeFlags();
        for (unsigned i = 0; i < (int)res_SerializedVertexAttribute::TOTAL_MESH_ATTRIBUTES; ++i) {
            if (attribFlags & (1 << i))
                attributes[numAttributes++] = res_SerializedVertexAttribute_GetVertexAttribute((res_SerializedVertexAttribute)i);
        }
        size_t stride = 0;
        for (size_t i = 0; i < numAttributes; ++i)
            stride += gfx_VertexAttributeType_GetSize(attributes[i].Type());
        return stride;
    }


    // ----------------------------------------------
    // res_Mesh
    // ----------------------------------------------
    static gfx_VertexLayout
    CreateVertexLayout(gfx_GraphicsAdapter* graphicsAdapter, uint16_t attribFlags)
    {
        gfx_VertexAttribute attributes[(int)res_SerializedVertexAttribute::TOTAL_MESH_ATTRIBUTES];
        size_t              numAttributes = 0;
        for (unsigned i = 0; i < (int)res_SerializedVertexAttribute::TOTAL_MESH_ATTRIBUTES; ++i) {
            if (attribFlags & (1 << i))
                attributes[numAttributes++] = res_SerializedVertexAttribute_GetVertexAttribute((res_SerializedVertexAttribute)i);
        }
        return gfx_VertexLayout(graphicsAdapter, attributes, numAttributes);
    }

    res_Mesh::res_Mesh(gfx_GraphicsAdapter*       graphicsAdapter,
                       const gfx_VertexAttribute* attributes,
                       unsigned                   numAttributes,
                       const void*                vertexData,
                       size_t                     vertexDataSize,
                       const unsigned*            indexData,
                       size_t                     numIndices)
        : m_vertexBuffer(graphicsAdapter, vertexData, vertexDataSize, gfx_BufferUsage::STATIC)
        , m_indexBuffer(graphicsAdapter, indexData, numIndices, gfx_BufferUsage::STATIC)
        , m_vertexLayout(graphicsAdapter, attributes, numAttributes)
        , m_numTriangles(numIndices / 3)
    {
        size_t stride = 0;
        for (unsigned i = 0; i < numAttributes; ++i)
            stride += gfx_VertexAttributeType_GetSize(attributes[i].Type());
        m_vertexStride = stride;
    }

    res_Mesh::res_Mesh(res_Mesh&& other) noexcept
        : m_vertexBuffer(std::move(other.m_vertexBuffer))
        , m_indexBuffer(std::move(other.m_indexBuffer))
        , m_vertexLayout(std::move(other.m_vertexLayout))
        , m_vertexStride(other.m_vertexStride)
        , m_numTriangles(other.m_numTriangles)
    {}

    res_Mesh::res_Mesh(pge::gfx_GraphicsAdapter* graphicsAdapter, const res_SerializedMesh& smesh)
        : m_vertexBuffer(graphicsAdapter, smesh.GetVertexData(), smesh.GetVertexDataSize(), gfx_BufferUsage::STATIC)
        , m_indexBuffer(graphicsAdapter, smesh.GetTriangleData(), smesh.GetNumTriangles() * 3 * sizeof(unsigned), gfx_BufferUsage::STATIC)
        , m_vertexLayout(CreateVertexLayout(graphicsAdapter, smesh.GetAttributeFlags()))
        , m_vertexStride(smesh.GetVertexStride())
        , m_numTriangles(smesh.GetNumTriangles())
    {}

    res_Mesh::res_Mesh(pge::gfx_GraphicsAdapter* graphicsAdapter, const char* path)
        : res_Mesh(graphicsAdapter, res_SerializedMesh(path))
    {}

    void
    res_Mesh::Bind() const
    {
        m_vertexLayout.Bind();
        m_vertexBuffer.Bind(0, m_vertexStride, 0);
        m_indexBuffer.Bind(0);
    }

    size_t
    res_Mesh::GetNumTriangles() const
    {
        return m_numTriangles;
    }



    // ---------------------------------
    // res_MeshCache
    // ---------------------------------
    res_MeshCache::res_MeshCache(gfx_GraphicsAdapter* graphicsAdapter)
        : m_graphicsAdapter(graphicsAdapter)
    {}

    res_Mesh*
    res_MeshCache::Load(const char* path)
    {
        auto it = m_meshMap.find(path);
        if (it == m_meshMap.end()) {
            m_meshMap.emplace(std::piecewise_construct, std::make_tuple(path), std::make_tuple(m_graphicsAdapter, path));
        }
        return &m_meshMap.at(path);
    }
} // namespace pge