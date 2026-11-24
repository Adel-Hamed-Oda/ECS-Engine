#include "ModelLoader.h"

#include <assimp/mesh.h>

std::shared_ptr<Mesh> ModelLoader::LoadAssimpMesh(const aiMesh* aiM) {
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

    return mesh;
}

std::vector<std::shared_ptr<Mesh>> ModelLoader::LoadMeshes(const std::string& filepath, ModelLoaderFlags flags) {
    ModelData model = Load(filepath, flags);
    return model.meshes;
}
