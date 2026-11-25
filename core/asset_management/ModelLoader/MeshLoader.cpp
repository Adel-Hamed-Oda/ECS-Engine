#include "ModelLoader.h"

#include <assimp/mesh.h>

namespace {

static glm::mat4 ConvertAssimpMatrix(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static void SetVertexBoneData(Mesh& mesh, unsigned int vertexID, int boneID, float weight) {
    if (weight <= 0.0f || vertexID >= mesh.boneIDs.size()) {
        return;
    }

    for (int i = 0; i < Mesh::MAX_BONE_INFLUENCE; ++i) {
        if (mesh.boneIDs[vertexID][i] == boneID) {
            mesh.boneWeights[vertexID][i] += weight;
            return;
        }
        if (mesh.boneIDs[vertexID][i] < 0) {
            mesh.boneIDs[vertexID][i] = boneID;
            mesh.boneWeights[vertexID][i] = weight;
            return;
        }
    }

    // If all 4 slots are full, replace the smallest weight if this bone has stronger influence
    int minIndex = 0;
    for (int i = 1; i < Mesh::MAX_BONE_INFLUENCE; ++i) {
        if (mesh.boneWeights[vertexID][i] < mesh.boneWeights[vertexID][minIndex]) {
            minIndex = i;
        }
    }
    if (weight > mesh.boneWeights[vertexID][minIndex]) {
        mesh.boneIDs[vertexID][minIndex] = boneID;
        mesh.boneWeights[vertexID][minIndex] = weight;
    }
}

static void ExtractBoneWeights(Mesh& mesh, const aiMesh* aiM, Skeleton* skeleton) {
    if (!aiM || !aiM->HasBones() || aiM->mNumVertices == 0) {
        return;
    }

    mesh.boneIDs.assign(aiM->mNumVertices, glm::ivec4(-1));
    mesh.boneWeights.assign(aiM->mNumVertices, glm::vec4(0.0f));

    for (unsigned int boneIndex = 0; boneIndex < aiM->mNumBones; ++boneIndex) {
        const aiBone* bone = aiM->mBones[boneIndex];
        if (!bone) continue;

        bool hasPositiveWeight = false;
        for (unsigned int w = 0; w < bone->mNumWeights; ++w) {
            if (bone->mWeights[w].mWeight > 0.0f) {
                hasPositiveWeight = true;
                break;
            }
        }
        if (!hasPositiveWeight) continue;

        int boneID = -1;
        std::string boneName = bone->mName.C_Str();

        if (skeleton) {
            auto it = skeleton->boneInfoMap.find(boneName);
            if (it == skeleton->boneInfoMap.end()) {
                if (skeleton->boneCount >= Mesh::MAX_BONES) {
                    continue;
                }
                BoneInfo newBoneInfo;
                newBoneInfo.id = skeleton->boneCount;
                newBoneInfo.offsetMatrix = ConvertAssimpMatrix(bone->mOffsetMatrix);
                skeleton->boneInfoMap[boneName] = newBoneInfo;
                boneID = newBoneInfo.id;
                skeleton->boneCount++;
            } else {
                boneID = it->second.id;
            }
        } else {
            if (static_cast<int>(boneIndex) >= Mesh::MAX_BONES) {
                continue;
            }
            boneID = static_cast<int>(boneIndex);
        }

        if (boneID < 0) continue;

        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
            unsigned int vertexId = bone->mWeights[weightIndex].mVertexId;
            float weight = bone->mWeights[weightIndex].mWeight;
            SetVertexBoneData(mesh, vertexId, boneID, weight);
        }
    }

    // Normalize weights per vertex so active bone weights sum to 1.0
    for (size_t v = 0; v < mesh.boneWeights.size(); ++v) {
        glm::vec4& w = mesh.boneWeights[v];
        float sum = w.x + w.y + w.z + w.w;
        if (sum > 0.00001f) {
            w /= sum;
        }
    }
}

} // anonymous namespace

std::shared_ptr<Mesh> ModelLoader::LoadAssimpMesh(const aiMesh* aiM, Skeleton* skeleton) {
    auto mesh = std::make_shared<Mesh>();
    mesh->vertices.reserve(aiM->mNumVertices);
    mesh->normals.reserve(aiM->mNumVertices);
    mesh->uvs.reserve(aiM->mNumVertices);
    mesh->colors.reserve(aiM->mNumVertices);
    mesh->tangents.reserve(aiM->mNumVertices);

    const bool hasNormals = aiM->HasNormals();
    const bool hasUVs = aiM->HasTextureCoords(0);
    const bool hasTangents = aiM->HasTangentsAndBitangents();

    // Ignore uninitialized all-black vertex color layers (common in FBX exports) so they don't zero out baseColor textures
    bool hasValidColors = false;
    if (aiM->HasVertexColors(0)) {
        for (unsigned int v = 0; v < aiM->mNumVertices; ++v) {
            const aiColor4D& c = aiM->mColors[0][v];
            if ((c.r + c.g + c.b) > 0.001f) {
                hasValidColors = true;
                break;
            }
        }
    }

    for (unsigned int v = 0; v < aiM->mNumVertices; ++v) {
        mesh->vertices.emplace_back(aiM->mVertices[v].x, aiM->mVertices[v].y, aiM->mVertices[v].z);

        if (hasNormals) {
            mesh->normals.emplace_back(aiM->mNormals[v].x, aiM->mNormals[v].y, aiM->mNormals[v].z);
        } else {
            mesh->normals.emplace_back(0.0f, 1.0f, 0.0f);
        }

        if (hasUVs) {
            mesh->uvs.emplace_back(aiM->mTextureCoords[0][v].x, aiM->mTextureCoords[0][v].y);
        } else {
            mesh->uvs.emplace_back(0.0f, 0.0f);
        }

        if (hasValidColors) {
            mesh->colors.emplace_back(aiM->mColors[0][v].r, aiM->mColors[0][v].g, aiM->mColors[0][v].b, aiM->mColors[0][v].a);
        } else {
            mesh->colors.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
        }

        if (hasTangents) {
            mesh->tangents.emplace_back(aiM->mTangents[v].x, aiM->mTangents[v].y, aiM->mTangents[v].z);
        } else {
            mesh->tangents.emplace_back(0.0f, 0.0f, 0.0f);
        }
    }

    for (unsigned int f = 0; f < aiM->mNumFaces; ++f) {
        const aiFace& face = aiM->mFaces[f];
        for (unsigned int i = 0; i < face.mNumIndices; ++i) {
            mesh->triangles.push_back(face.mIndices[i]);
        }
    }

    if (aiM->HasBones()) {
        ExtractBoneWeights(*mesh, aiM, skeleton);
    }

    return mesh;
}

std::vector<std::shared_ptr<Mesh>> ModelLoader::LoadMeshes(const std::string& filepath, ModelLoaderFlags flags) {
    ModelData model = Load(filepath, flags);
    return model.meshes;
}
