#include "ModelLoader.h"

#include <assimp/scene.h>
#include <assimp/material.h>

#include <algorithm>

std::shared_ptr<DefaultMaterial> ModelLoader::LoadAssimpMaterial(
    const aiScene* scene,
    const aiMaterial* aiMat,
    const std::string& directory,
    std::unordered_map<std::string, std::shared_ptr<Texture>>& cache
) {
    auto material = std::make_shared<DefaultMaterial>();

    aiColor4D diffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
        material->baseColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
    } else if (aiMat->Get(AI_MATKEY_BASE_COLOR, diffuseColor) == AI_SUCCESS) {
        material->baseColor = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
    }

    aiColor3D ambientColor(0.1f, 0.1f, 0.1f);
    if (aiMat->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor) == AI_SUCCESS) {
        material->ambient = glm::vec3(ambientColor.r, ambientColor.g, ambientColor.b);
    }

    float shininess = 32.0f;
    if (aiMat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        material->shininess = std::max(1.0f, shininess);
    }

    aiColor3D specularColor(0.5f, 0.5f, 0.5f);
    if (aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS) {
        material->specularStrength = (specularColor.r + specularColor.g + specularColor.b) / 3.0f;
    }

    // 1. Base Color / Diffuse Map
    std::shared_ptr<Texture> tex = LoadMaterialTexture(scene, aiMat, aiTextureType_DIFFUSE, directory, cache);
    if (!tex) {
        tex = LoadMaterialTexture(scene, aiMat, aiTextureType_BASE_COLOR, directory, cache);
    }
    if (tex) {
        material->texture = tex.get();
    }

    // 2. Normal Map (OBJ/MTL map_Bump is often mapped to HEIGHT or NORMALS by Assimp)
    std::shared_ptr<Texture> normTex = LoadMaterialTexture(scene, aiMat, aiTextureType_NORMALS, directory, cache);
    if (!normTex) {
        normTex = LoadMaterialTexture(scene, aiMat, aiTextureType_NORMAL_CAMERA, directory, cache);
    }
    if (!normTex) {
        normTex = LoadMaterialTexture(scene, aiMat, aiTextureType_HEIGHT, directory, cache);
    }
    if (normTex) {
        material->normalMap = normTex.get();
    }

    // 3. Roughness / Specular Map
    std::shared_ptr<Texture> roughTex = LoadMaterialTexture(scene, aiMat, aiTextureType_DIFFUSE_ROUGHNESS, directory, cache);
    if (!roughTex) {
        roughTex = LoadMaterialTexture(scene, aiMat, aiTextureType_SPECULAR, directory, cache);
    }
    if (!roughTex) {
        roughTex = LoadMaterialTexture(scene, aiMat, aiTextureType_SHININESS, directory, cache);
    }
    if (!roughTex) {
        roughTex = LoadMaterialTexture(scene, aiMat, aiTextureType_UNKNOWN, directory, cache);
    }
    if (roughTex) {
        material->roughnessMap = roughTex.get();
    }

    return material;
}

std::vector<std::shared_ptr<DefaultMaterial>> ModelLoader::LoadMaterials(const std::string& filepath) {
    ModelData model = Load(filepath);
    return model.materials;
}
