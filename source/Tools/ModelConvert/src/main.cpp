#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdio.h>
#include <res_mesh.h>
#include <gfx_vertex_layout.h>
#include <diag_assert.h>
#include <fstream>
#include <sstream>
#include <Windows.h>

void
ExtractMesh(const aiMesh* mesh, const char* targetPath)
{
    using namespace pge;

    uint16_t attributeFlags = 0;
    attributeFlags |= mesh->HasPositions() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::POSITION) : 0;
    attributeFlags |= mesh->HasNormals() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::NORMAL) : 0;
    attributeFlags |= mesh->HasTextureCoords(0) ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::TEXTURECOORD) : 0;
    attributeFlags |= mesh->HasVertexColors(0) ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::COLOR) : 0;

    const size_t vertexStride     = res_SerializedVertexAttribute_GetVertexStride(attributeFlags);
    const size_t vertexDataSize   = mesh->mNumVertices * vertexStride;
    const size_t triangleDataSize = mesh->mNumVertices * 3 * sizeof(unsigned);

    std::unique_ptr<unsigned[]> triangleData(new unsigned[triangleDataSize]);
    unsigned*                   triangleDataIt = triangleData.get();
    diag_Assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            *triangleDataIt = mesh->mFaces[i].mIndices[j];
            triangleDataIt++;
        }
    }

    std::stringstream ss;
    CreateDirectory(targetPath, nullptr);
    ss << targetPath << "\\meshes";
    CreateDirectory(ss.str().c_str(), nullptr);
    ss << "\\" << mesh->mName.C_Str() << ".mesh";

    std::ofstream      output(ss.str().c_str(), std::ios::binary);
    res_SerializedMesh model(reinterpret_cast<math_Vec3*>(mesh->mVertices),
                             reinterpret_cast<math_Vec3*>(mesh->mNormals),
                             reinterpret_cast<math_Vec2*>(mesh->mTextureCoords[0]),
                             reinterpret_cast<math_Vec3*>(mesh->mColors[0]),
                             mesh->mNumVertices,
                             triangleData.get(),
                             mesh->mNumFaces);

    model.Write(output);
    output.close();
}

void
ExtractMaterial(const aiMaterial* material, const char* targetPath)
{}

void
ExtractSkeleton(const aiNode* rootNode, const char* targetPath)
{}

void
ExtractAnimation(const aiAnimation* animation, const char* targetPath)
{}

void
ConvertModel(const char* sourcePath, const char* targetPath)
{
    unsigned importFlags = 0;
    //    importFlags |= aiProcess_CalcTangentSpace;
    importFlags |= aiProcess_Triangulate;
    //    importFlags |= aiProcess_JoinIdenticalVertices;
    //    importFlags |= aiProcess_SortByPType;
    const aiScene* scene = aiImportFile(sourcePath, importFlags);
    if (scene == nullptr) {
        printf("Could not open the scene at path: %s", sourcePath);
        printf("Aborting conversion for: %s", sourcePath);
        return;
    }

    auto SceneHasSkeleton = [=](const aiScene* scene) {
        for (unsigned i = 0; i < scene->mNumMeshes; ++i)
            if (scene->mMeshes[i]->HasBones())
                return true;
        return false;
    };

    printf("Found %d mesh(es).\n", scene->mNumMeshes);
    printf("Found %d material(s).\n", scene->mNumMaterials);
    printf("Found %s skeleton.\n", SceneHasSkeleton(scene) ? "a" : "no");
    printf("Found %d animation(s).\n", scene->mNumAnimations);

    for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
        ExtractMesh(scene->mMeshes[i], targetPath);
        printf("Done extracting mesh %d: %s\n", i+1, scene->mMeshes[i]->mName.C_Str());
    }
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        ExtractMaterial(scene->mMaterials[i], targetPath);
        printf("Done extracting material %d: %s\n", i+1, scene->mMaterials[i]->GetName().C_Str());
    }
    if (SceneHasSkeleton(scene)) {
        ExtractSkeleton(scene->mRootNode, targetPath);
        printf("Done extracting skeleton\n");
    }
    for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
        ExtractAnimation(scene->mAnimations[i], targetPath);
        printf("Done extracting animation %d: %s\n", i+1, scene->mAnimations[i]->mName.C_Str());
    }

    aiReleaseImport(scene);
}

int
main()
{
    ConvertModel(R"(C:\Users\phili\Desktop\untitled1.fbx)", R"(C:\Users\phili\Desktop\untitled1)");
    return 0;
}