#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <core_assert.h>
#include <core_file_utils.h>
#include <gfx_vertex_layout.h>
#include <res_mesh.h>
#include <res_material.h>

#include <stdio.h>
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
    core_Assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        for (int j = 0; j < 3; ++j) {
            *triangleDataIt = mesh->mFaces[i].mIndices[j];
            triangleDataIt++;
        }
    }

    std::stringstream ss;
    CreateDirectory(targetPath, nullptr);
    ss << targetPath << "\\" << mesh->mName.C_Str() << ".mesh";

    std::vector<math_Vec2> texcoords;
    texcoords.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& texcoord = mesh->mTextureCoords[0][i];
        texcoords.emplace_back(math_Vec2(texcoord.x, texcoord.y));
    }

    // Pre-transform the vertices
    aiMatrix4x4 rotation;
    aiMatrix4x4::RotationX(0.5f * math_PI, rotation);

    aiMatrix4x4 scale;
    aiMatrix4x4::Scaling(aiVector3t<float>(2, 2, 2), scale);

    aiMatrix4x4             importTransform = rotation * scale;
    std::vector<aiVector3D> importVertices, importNormals;
    importVertices.reserve(mesh->mNumVertices);
    importNormals.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D pos = importTransform * mesh->mVertices[i];
        importVertices.emplace_back(pos);

        const aiVector3D norm = importTransform * mesh->mNormals[i];
        importNormals.emplace_back(norm);
    }

    // Write to output
    std::ofstream      output(ss.str().c_str(), std::ios::binary);
    res_SerializedMesh model(reinterpret_cast<math_Vec3*>(&importVertices[0]),
                             reinterpret_cast<math_Vec3*>(&importNormals[0]),
                             &texcoords[0],
                             reinterpret_cast<math_Vec3*>(mesh->mColors[0]),
                             mesh->mNumVertices,
                             triangleData.get(),
                             mesh->mNumFaces);

    model.Write(output);
    output.close();
}

void
ExtractMaterial(aiMaterial* material, const char* targetPath)
{
    auto FindFirstOccuranceInString = [](const char* str, const char* substr) {
        size_t substrLen = strlen(substr);
        size_t matches = 0;
        for (const char* p = str; *p != '\0'; ++p) {
            if (*p == substr[matches]) {
                ++matches;
            } else {
                matches = 0;
            }
            if (matches == substrLen) {
                for (size_t i = 0; i < (matches-1); ++i)
                    --p;
                return p;
            }
        }
        return str;
    };

    const char* localTargetPath = FindFirstOccuranceInString(targetPath, "data\\");

    std::stringstream matFileSrc;
    matFileSrc << "Effect = data\\effects\\default.effect\n"
                  "Properties {\n"
                  "  float4    MainColor(1, 1, 1, 1)\n";
    aiString texPath;
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS) {
        std::stringstream localTexPath;
        localTexPath << localTargetPath << "\\" << pge::core_GetFilenameFromPath(texPath.C_Str());
        matFileSrc << "  Texture2D DiffuseMap(" << localTexPath.str() << ")\n";
    }
    matFileSrc << "}";


    std::stringstream matPath;
    matPath << targetPath << "\\" << material->GetName().C_Str() << ".mat";
    std::ofstream mat(matPath.str());
    mat << matFileSrc.str();
    mat.close();
}

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
    importFlags |= aiProcess_FlipUVs;
    // importFlags |= aiProcess_ConvertToLeftHanded;
    // importFlags &= ~aiProcess_FlipWindingOrder;
    //     importFlags |= aiProcess_JoinIdenticalVertices;
    //     importFlags |= aiProcess_SortByPType;
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
        printf("Done extracting mesh %d: %s\n", i + 1, scene->mMeshes[i]->mName.C_Str());
    }
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        ExtractMaterial(scene->mMaterials[i], targetPath);
        printf("Done extracting material %d: %s\n", i + 1, scene->mMaterials[i]->GetName().C_Str());
    }
    if (SceneHasSkeleton(scene)) {
        ExtractSkeleton(scene->mRootNode, targetPath);
        printf("Done extracting skeleton\n");
    }
    for (unsigned i = 0; i < scene->mNumAnimations; ++i) {
        ExtractAnimation(scene->mAnimations[i], targetPath);
        printf("Done extracting animation %d: %s\n", i + 1, scene->mAnimations[i]->mName.C_Str());
    }

    aiReleaseImport(scene);
}

int
main()
{
    const char* inPath  = R"(C:\Users\phili\Desktop\Dungeon Pack)";
    const char* outPath = R"(D:\Projects\pge\data\Dungeon Pack Export)";
    auto        models  = pge::core_FSItemsWithExtension(inPath, "fbx", false);
    for (const auto& modelItem : models) {
        if (modelItem.type != pge::core_FSItemType::FILE)
            continue;
        const char* modelPath = modelItem.path.c_str();
        ConvertModel(modelPath, outPath);
    }
    return 0;
}