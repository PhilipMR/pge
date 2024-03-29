#include "../include/res_mesh.h"
#include <core_assert.h>

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
        : m_path(path)
    {
        std::ifstream input(path, std::ios::binary);
        core_Assert(input.is_open());
        input.read((char*)&m_version, sizeof(m_version));
        input.read((char*)&m_attributeFlags, sizeof(m_attributeFlags));
        input.read((char*)&m_numVertices, sizeof(m_numVertices));
        input.read((char*)&m_vertexDataSize, sizeof(m_vertexDataSize));
        input.read((char*)&m_numTriangles, sizeof(m_numTriangles));

        m_vertexData   = std::unique_ptr<char[]>(new char[m_vertexDataSize]);
        m_triangleData = std::unique_ptr<unsigned[]>(new unsigned[m_numTriangles * 3]);
        input.read(m_vertexData.get(), m_vertexDataSize);
        input.read((char*)&m_triangleData[0], GetTriangleDataSize(m_numTriangles));

        unsigned numBones = 0;
        input.read((char*)&numBones, sizeof(numBones));
        if (numBones > 0) {
            m_boneOffsetMatrices.resize(numBones);
            input.read((char*)&m_boneOffsetMatrices[0], numBones * sizeof(m_boneOffsetMatrices[0]));
        }

        input.close();
    }

    res_SerializedMesh::res_SerializedMesh(math_Vec3*         positions,
                                           math_Vec3*         normals,
                                           math_Vec2*         texcoords,
                                           math_Vec3*         colors,
                                           math_Vec4*         boneWeights,
                                           math_Vec4i*        boneIndices,
                                           size_t             numVertices,
                                           unsigned*          triangleData,
                                           size_t             numTriangles,
                                           const math_Mat4x4* boneOffsetMatrices,
                                           unsigned           numBones)
        : m_path("<from-memory>")
        , m_version(res_SerializedMesh::Version)
        , m_numVertices(numVertices)
        , m_numTriangles(numTriangles)
    {
        m_attributeFlags = 0;
        m_attributeFlags |= !positions ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::POSITION);
        m_attributeFlags |= !normals ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::NORMAL);
        m_attributeFlags |= !texcoords ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::TEXTURECOORD);
        m_attributeFlags |= !colors ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::COLOR);
        m_attributeFlags |= !boneWeights ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::BONEWEIGHTS);
        m_attributeFlags |= !boneIndices ? 0 : res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::BONEINDICES);

        size_t stride    = res_SerializedVertexAttribute_GetVertexStride(m_attributeFlags);
        m_vertexDataSize = stride * numVertices;

        m_vertexData   = std::unique_ptr<char[]>(new char[m_vertexDataSize]);
        m_triangleData = std::unique_ptr<unsigned[]>(new unsigned[m_numTriangles * 3]);

        if (numBones > 0) {
            m_boneOffsetMatrices.resize(numBones);
            memcpy(&m_boneOffsetMatrices[0], boneOffsetMatrices, numBones * sizeof(math_Mat4x4));
        }

        size_t offset = 0;
        for (size_t i = 0; i < numVertices; ++i) {
            if (positions) {
                memcpy(m_vertexData.get() + offset, &positions[i], sizeof(math_Vec3));
                offset += sizeof(math_Vec3);
            }
            if (normals) {
                memcpy(m_vertexData.get() + offset, &normals[i], sizeof(math_Vec3));
                offset += sizeof(math_Vec3);
            }
            if (texcoords) {
                memcpy(m_vertexData.get() + offset, &texcoords[i], sizeof(math_Vec2));
                offset += sizeof(math_Vec2);
            }
            if (colors) {
                memcpy(m_vertexData.get() + offset, &colors[i], sizeof(math_Vec3));
                offset += sizeof(math_Vec3);
            }
            if (boneWeights) {
                memcpy(m_vertexData.get() + offset, &boneWeights[i], sizeof(math_Vec4));
                offset += sizeof(math_Vec4);
            }
            if (boneIndices) {
                memcpy(m_vertexData.get() + offset, &boneIndices[i], sizeof(math_Vec4i));
                offset += sizeof(math_Vec4i);
            }
        }
        memcpy(m_triangleData.get(), triangleData, GetTriangleDataSize(m_numTriangles));
    }

    void
    res_SerializedMesh::Write(std::ostream& output) const
    {
        output.write((const char*)&m_version, sizeof(m_version));
        output.write((const char*)&m_attributeFlags, sizeof(m_attributeFlags));
        output.write((const char*)&m_numVertices, sizeof(m_numVertices));
        output.write((const char*)&m_vertexDataSize, sizeof(m_vertexDataSize));
        output.write((const char*)&m_numTriangles, sizeof(m_numTriangles));
        output.write((const char*)m_vertexData.get(), m_vertexDataSize);
        output.write((const char*)m_triangleData.get(), GetTriangleDataSize(m_numTriangles));

        unsigned numBones = m_boneOffsetMatrices.size();
        output.write((const char*)&numBones, sizeof(numBones));
        if (numBones > 0) {
            output.write((const char*)&m_boneOffsetMatrices[0], numBones * sizeof(m_boneOffsetMatrices[0]));
        }
    }

    std::string
    res_SerializedMesh::GetPath() const
    {
        return m_path;
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
        return m_vertexDataSize / m_numVertices;
    }

    math_AABB
    res_SerializedMesh::GetAABB() const
    {
        size_t vertexStride = GetVertexStride();
        return math_AABB(m_vertexData.get(), m_numVertices, vertexStride, 0);
    }

    const std::vector<math_Mat4x4>&
    res_SerializedMesh::GetBoneOffsetMatrices() const
    {
        return m_boneOffsetMatrices;
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
                       size_t                     numAttributes,
                       const void*                vertexData,
                       size_t                     vertexDataSize,
                       const unsigned*            indexData,
                       size_t                     numIndices,
                       const math_Mat4x4*         boneOffsetMatrices,
                       unsigned                   numBones)
        : m_path("<from-memory>")
        , m_vertexBuffer(graphicsAdapter, vertexData, vertexDataSize, gfx_BufferUsage::STATIC)
        , m_indexBuffer(graphicsAdapter, indexData, numIndices * sizeof(unsigned), gfx_BufferUsage::STATIC)
        , m_vertexLayout(graphicsAdapter, attributes, numAttributes)
        , m_numTriangles(numIndices / 3)
    {
        size_t stride = 0;
        for (size_t i = 0; i < numAttributes; ++i)
            stride += gfx_VertexAttributeType_GetSize(attributes[i].Type());
        m_vertexStride = stride;

        // Determine the mesh's local AABB
        core_AssertWithReason(strcmp(attributes[0].Name(), "POSITION") == 0, "The position has to be the first attribute!");
        size_t numVertices = vertexDataSize / m_vertexStride;
        m_aabb             = math_AABB(reinterpret_cast<const char*>(vertexData), numVertices, m_vertexStride, 0);

        // Copy the bone offset matrices
        m_boneMatrices.resize(numBones);
        for (unsigned i = 0; i < numBones; ++i)
            m_boneMatrices[i] = boneOffsetMatrices[i];
    }

    res_Mesh::res_Mesh(res_Mesh&& other) noexcept
        : m_path(other.m_path)
        , m_vertexBuffer(std::move(other.m_vertexBuffer))
        , m_indexBuffer(std::move(other.m_indexBuffer))
        , m_vertexLayout(std::move(other.m_vertexLayout))
        , m_vertexStride(other.m_vertexStride)
        , m_numTriangles(other.m_numTriangles)
        , m_aabb(other.m_aabb)
        , m_boneMatrices(std::move(other.m_boneMatrices))
    {}

    res_Mesh::res_Mesh(pge::gfx_GraphicsAdapter* graphicsAdapter, const res_SerializedMesh& smesh)
        : m_path(smesh.GetPath())
        , m_vertexBuffer(graphicsAdapter, smesh.GetVertexData(), smesh.GetVertexDataSize(), gfx_BufferUsage::STATIC)
        , m_indexBuffer(graphicsAdapter, smesh.GetTriangleData(), smesh.GetNumTriangles() * 3 * sizeof(unsigned), gfx_BufferUsage::STATIC)
        , m_vertexLayout(CreateVertexLayout(graphicsAdapter, smesh.GetAttributeFlags()))
        , m_vertexStride(smesh.GetVertexStride())
        , m_numTriangles(smesh.GetNumTriangles())
        , m_aabb(smesh.GetAABB())
        , m_boneMatrices(smesh.GetBoneOffsetMatrices())
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

    math_AABB
    res_Mesh::GetAABB() const
    {
        return m_aabb;
    }

    std::string
    res_Mesh::GetPath() const
    {
        return m_path;
    }

    const std::vector<math_Mat4x4>&
    res_Mesh::GetBoneOffsetMatrices() const
    {
        return m_boneMatrices;
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