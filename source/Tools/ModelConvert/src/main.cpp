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

    const size_t vertexStride = res_SerializedVertexAttribute_GetVertexStride(attributeFlags);
    const size_t vertexDataSize = mesh->mNumVertices * vertexStride;
    const size_t triangleDataSize = mesh->mNumVertices * 3 * sizeof(unsigned);

    char* vertexData    = (char*)malloc(vertexDataSize);
    char* vertexDataBegin = vertexData;

    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        if (mesh->HasPositions()) {
            const aiVector3D& position = mesh->mVertices[i];
            memcpy(vertexData, &position, sizeof(position));
            vertexData += sizeof(position);
        }
        if (mesh->HasNormals()) {
            const aiVector3D& normal = mesh->mNormals[i];
            memcpy(vertexData, &normal, sizeof(normal));
            vertexData += sizeof(normal);
        }
        if (mesh->HasTextureCoords(0)) {
            const aiVector2D& texcoord0 = aiVector2D(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            memcpy(vertexData, &texcoord0, sizeof(texcoord0));
            vertexData += sizeof(texcoord0);
        }
        if (mesh->HasVertexColors(0)) {
            const aiVector3D& color = aiVector3D(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
            memcpy(vertexData, &color, sizeof(color));
            vertexData += sizeof(color);
        }
    }

    unsigned* triangleData = (unsigned*)malloc(triangleDataSize);
    unsigned* triangleDataBegin = triangleData;
    diag_Assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            *triangleData = mesh->mFaces[i].mIndices[j];
            triangleData++;
        }
    }

    std::stringstream ss;
    CreateDirectory(targetPath, nullptr);
    ss << targetPath << "\\meshes";
    CreateDirectory(ss.str().c_str(), nullptr);
    ss << "\\" << mesh->mName.C_Str() << ".mesh";

    std::ofstream output(ss.str().c_str(), std::ios::binary);
    res_SerializedMesh model(1, attributeFlags, mesh->mNumVertices, vertexDataSize, mesh->mNumFaces, vertexDataBegin, triangleDataBegin);
    model.Write(output);
    output.close();
    free(vertexDataBegin);
    free(triangleDataBegin);
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
        printf("Done extracting mesh %d: %s\n", i, scene->mMeshes[i]->mName.C_Str());
    }
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        ExtractMaterial(scene->mMaterials[i], targetPath);
        printf("Done extracting material %d: %s\n", i, scene->mMaterials[i]->GetName().C_Str());
    }
    if (SceneHasSkeleton(scene)) {
        ExtractSkeleton(scene->mRootNode, targetPath);
        printf("Done extracting skeleton\n");
    }
    for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
        ExtractAnimation(scene->mAnimations[i], targetPath);
        printf("Done extracting animation %d: %s\n", i, scene->mAnimations[i]->mName.C_Str());
    }

    aiReleaseImport(scene);
}

int
main()
{
    ConvertModel("C:\\Users\\phili\\Desktop\\suzanne.fbx", "C:\\Users\\phili\\Desktop\\suzanne");
    return 0;
}