#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <core_assert.h>
#include <core_file_utils.h>
#include <gfx_vertex_layout.h>
#include <res_mesh.h>
#include <res_material.h>
#include <res_skeleton.h>

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <Windows.h>

void
ExtractMesh(const aiMesh* mesh, const char* targetPath, const pge::anim_Skeleton* skeleton)
{
    using namespace pge;

    uint16_t attributeFlags = 0;
    attributeFlags |= mesh->HasPositions() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::POSITION) : 0;
    attributeFlags |= mesh->HasNormals() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::NORMAL) : 0;
    attributeFlags |= mesh->HasTextureCoords(0) ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::TEXTURECOORD) : 0;
    attributeFlags |= mesh->HasVertexColors(0) ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::COLOR) : 0;
    attributeFlags |= mesh->HasBones() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::BONEWEIGHTS) : 0;
    attributeFlags |= mesh->HasBones() ? res_SerializedVertexAttribute_GetFlag(res_SerializedVertexAttribute::BONEINDICES) : 0;

    // Either no bones and no skeleton, or both
    core_Assert(!mesh->HasBones() || skeleton != nullptr);

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
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& texcoord = mesh->mTextureCoords[0][i];
        texcoords.emplace_back(math_Vec2(texcoord.x, texcoord.y));
    }

    // Pre-transform the vertices
    aiMatrix4x4 rotation;
    aiMatrix4x4::RotationX(0.5f * math_PI, rotation);

    aiMatrix4x4 scale;
    aiMatrix4x4::Scaling(aiVector3t<float>(2, 2, 2), scale);

    aiMatrix4x4             importTransform = aiMatrix4x4(); //rotation * scale;
    std::vector<aiVector3D> importVertices, importNormals;
    importVertices.reserve(mesh->mNumVertices);
    importNormals.reserve(mesh->mNumVertices);
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D pos = importTransform * mesh->mVertices[i];
        importVertices.emplace_back(pos);

        const aiVector3D norm = importTransform * mesh->mNormals[i];
        importNormals.emplace_back(norm);
    }

    std::vector<aiMatrix4x4> boneOffsetMatrices;
    std::vector<math_Vec4>   boneWeights;
    std::vector<math_Vec4i>  boneIndices;
    if (skeleton != nullptr) {
        boneOffsetMatrices.resize(skeleton->GetBoneCount());

        boneWeights.resize(mesh->mNumVertices);
        boneIndices.resize(mesh->mNumVertices);
        std::for_each(boneIndices.begin(), boneIndices.end(), [](math_Vec4i& idx) { idx = math_Vec4i(-1, -1, -1, -1); });

        for (unsigned i = 0; i < mesh->mNumBones; ++i) {
            const aiBone* aiBone    = mesh->mBones[i];
            const int     boneIndex = skeleton->GetBoneIndex(aiBone->mName.C_Str());
            core_Assert(boneIndex >= 0);
            core_Assert(boneIndex < skeleton->GetBoneCount());

            boneOffsetMatrices[boneIndex] = aiBone->mOffsetMatrix;

            for (unsigned j = 0; j < aiBone->mNumWeights; ++j) {
                const aiVertexWeight& aiWeight = aiBone->mWeights[j];

                const unsigned vertexIdx   = aiWeight.mVertexId;
                int            freeBoneIdx = -1;
                for (int k = 0; k < 4; ++k) {
                    if (boneIndices[vertexIdx][k] != -1)
                        continue;
                    freeBoneIdx = k;
                    k           = 4; // Break out of inner loop
                }
                core_Assert(freeBoneIdx != -1);
                boneIndices[vertexIdx][freeBoneIdx] = boneIndex;
                boneWeights[vertexIdx][freeBoneIdx] = aiWeight.mWeight;
            }
        }
    }


    // Write to output
    std::ofstream      output(ss.str().c_str(), std::ios::binary);
    res_SerializedMesh model(reinterpret_cast<math_Vec3*>(&importVertices[0]),
                             reinterpret_cast<math_Vec3*>(&importNormals[0]),
                             &texcoords[0],
                             reinterpret_cast<math_Vec3*>(mesh->mColors[0]),
                             skeleton == nullptr ? nullptr : &boneWeights[0],
                             skeleton == nullptr ? nullptr : &boneIndices[0],
                             mesh->mNumVertices,
                             triangleData.get(),
                             mesh->mNumFaces,
                             skeleton == nullptr ? nullptr : reinterpret_cast<math_Mat4x4*>(&boneOffsetMatrices[0]),
                             boneOffsetMatrices.size());

    model.Write(output);
    output.close();
}

void
ExtractTexture(aiTexture* texture, const char* targetPath)
{
    std::string       texName = pge::core_GetFilenameFromPath(texture->mFilename.C_Str());
    std::stringstream texPath;
    texPath << targetPath << "\\" << texName;

    // If mHeight == 0, then it is a compressed format (and the data is stored directly)
    if (texture->mHeight == 0) {
        std::ofstream tex(texPath.str(), std::ios::binary);
        tex.write((const char*)texture->pcData, texture->mWidth);
    } else {
        stbi_write_png(texPath.str().c_str(), texture->mWidth, texture->mHeight, 4, texture->pcData, sizeof(aiTexel));
    }
}

void
ExtractMaterial(aiMaterial* material, const char* targetPath)
{
    auto FindFirstOccuranceInString = [](const char* str, const char* substr) {
        size_t substrLen = strlen(substr);
        size_t matches   = 0;
        for (const char* p = str; *p != '\0'; ++p) {
            if (*p == substr[matches]) {
                ++matches;
            } else {
                matches = 0;
            }
            if (matches == substrLen) {
                for (size_t i = 0; i < (matches - 1); ++i)
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
ExtractBoneData(std::vector<pge::res_Bone>* data, const aiNode* node, int parentIdx)
{
    pge::res_Bone bone;
    bone.name = node->mName.C_Str();
    memcpy(&bone.transform, &node->mTransformation, sizeof(bone.transform));
    bone.parent = parentIdx;
    data->push_back(bone);
    const int index = data->size() - 1;
    for (size_t i = 0; i < node->mNumChildren; ++i) {
        ExtractBoneData(data, node->mChildren[i], index);
    }
}

pge::res_Skeleton
ExtractSkeleton(const aiNode* rootNode, const char* targetPath)
{
    std::vector<pge::res_Bone> boneData;
    ExtractBoneData(&boneData, rootNode, -1);
    pge::res_Skeleton skeleton(&boneData[0], boneData.size());
    std::stringstream ss;
    ss << targetPath << "\\" << boneData[0].name << ".skel";
    std::ofstream skel(ss.str(), std::ios::binary);
    skeleton.Write(skel);
    return skeleton;
}

static pge::math_Vec3
Vec3FromAssimp(const aiVector3D& vec)
{
    return pge::math_Vec3(vec.x, vec.y, vec.z);
}

static pge::math_Quat
QuatFromAssimp(const aiQuaternion& quat)
{
    return pge::math_Quat(quat.w, quat.x, quat.y, quat.z);
}

void
ExtractAnimation(const aiAnimation* animation, const char* targetPath)
{
    std::vector<pge::anim_SkeletonAnimationChannel> channels;
    channels.reserve(animation->mNumChannels);
    for (unsigned i = 0; i < animation->mNumChannels; ++i) {
        aiNodeAnim* nodeAnim = animation->mChannels[i];

        const char*                    boneName = nodeAnim->mNodeName.C_Str();
        std::vector<pge::anim_KeyVec3> positionKeys;
        std::vector<pge::anim_KeyVec3> scaleKeys;
        std::vector<pge::anim_KeyQuat> rotationKeys;

        positionKeys.reserve(nodeAnim->mNumPositionKeys);
        for (unsigned j = 0; j < nodeAnim->mNumPositionKeys; ++j) {
            pge::anim_KeyVec3 key;
            key.time  = nodeAnim->mPositionKeys[j].mTime / animation->mTicksPerSecond;
            key.value = Vec3FromAssimp(nodeAnim->mPositionKeys[j].mValue);
            positionKeys.emplace_back(key);
        }

        scaleKeys.reserve(nodeAnim->mNumScalingKeys);
        for (unsigned j = 0; j < nodeAnim->mNumScalingKeys; ++j) {
            pge::anim_KeyVec3 key;
            key.time  = nodeAnim->mScalingKeys[j].mTime / animation->mTicksPerSecond;
            key.value = Vec3FromAssimp(nodeAnim->mScalingKeys[j].mValue);
            scaleKeys.emplace_back(key);
        }

        rotationKeys.reserve(nodeAnim->mNumRotationKeys);
        for (unsigned j = 0; j < nodeAnim->mNumRotationKeys; ++j) {
            pge::anim_KeyQuat key;
            key.time  = nodeAnim->mRotationKeys[j].mTime / animation->mTicksPerSecond;
            key.value = QuatFromAssimp(nodeAnim->mRotationKeys[j].mValue);
            rotationKeys.emplace_back(key);
        }

        channels.emplace_back(pge::anim_SkeletonAnimationChannel(boneName,
                                                                 &positionKeys[0],
                                                                 positionKeys.size(),
                                                                 &scaleKeys[0],
                                                                 scaleKeys.size(),
                                                                 &rotationKeys[0],
                                                                 rotationKeys.size()));
    }
    pge::res_SkeletonAnimation anim(animation->mName.C_Str(), animation->mDuration / animation->mTicksPerSecond, &channels[0], channels.size());

    std::stringstream ss;
    ss << targetPath << "\\" << animation->mName.C_Str() << ".skelanim";
    std::ofstream animFile(ss.str(), std::ios::binary);
    anim.Write(animFile);
}

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
    importFlags |= aiProcess_LimitBoneWeights;
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
    printf("Found %d texture(s).\n", scene->mNumTextures);
    printf("Found %d material(s).\n", scene->mNumMaterials);
    printf("Found %s skeleton.\n", SceneHasSkeleton(scene) ? "a" : "no");
    printf("Found %d animation(s).\n", scene->mNumAnimations);

    std::unique_ptr<pge::res_Skeleton> skeleton;
    if (SceneHasSkeleton(scene)) {
        skeleton = std::make_unique<pge::res_Skeleton>(ExtractSkeleton(scene->mRootNode, targetPath));
        printf("Done extracting skeleton\n");
    }
    for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
        ExtractMesh(scene->mMeshes[i], targetPath, skeleton.get() == nullptr ? nullptr : skeleton->GetSkeleton());
        printf("Done extracting mesh %d: %s\n", i + 1, scene->mMeshes[i]->mName.C_Str());
    }
    for (unsigned i = 0; i < scene->mNumTextures; ++i) {
        ExtractTexture(scene->mTextures[i], targetPath);
        printf("Done extracting texture %d: %s\n", i + 1, scene->mTextures[i]->mFilename.C_Str());
    }
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        ExtractMaterial(scene->mMaterials[i], targetPath);
        printf("Done extracting material %d: %s\n", i + 1, scene->mMaterials[i]->GetName().C_Str());
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
//    const char* inPath  = R"(C:\Users\phili\Desktop\Dungeon Pack)";
//    const char* outPath = R"(D:\Projects\pge\data\Dungeon Pack Export)";
//    auto        models  = pge::core_FSItemsWithExtension(inPath, "fbx", false);
//    for (const auto& modelItem : models) {
//        if (modelItem.type != pge::core_FSItemType::FILE)
//            continue;
//        const char* modelPath = modelItem.path.c_str();
//        ConvertModel(modelPath, outPath);
//    }

    ConvertModel(R"(C:\Users\phili\Desktop\Walking.fbx)", R"(C:\Users\phili\Desktop\Walking)");
//    ConvertModel(R"(C:\Users\phili\Desktop\Idle.fbx)", R"(D:\Projects\pge\data\Vampire\Idle)");
    return 0;
}