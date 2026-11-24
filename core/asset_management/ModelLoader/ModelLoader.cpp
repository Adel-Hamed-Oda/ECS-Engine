#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include <iostream>
#include <unordered_map>
#include <algorithm>

#include "core/asset_management/FileReader.h"

namespace {

static std::string GetDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(0, lastSlash + 1);
    }
    return "";
}

static std::string GetFilename(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(lastSlash + 1);
    }
    return filepath;
}

static unsigned int ConvertFlags(ModelLoaderFlags flags) {
    unsigned int aiFlags = 0;
    if (flags & ModelLoaderFlags::Triangulate)           aiFlags |= aiProcess_Triangulate;
    if (flags & ModelLoaderFlags::GenSmoothNormals)      aiFlags |= aiProcess_GenSmoothNormals;
    if (flags & ModelLoaderFlags::GenNormals)            aiFlags |= aiProcess_GenNormals;
    if (flags & ModelLoaderFlags::FlipUVs)               aiFlags |= aiProcess_FlipUVs;
    if (flags & ModelLoaderFlags::CalcTangentSpace)      aiFlags |= aiProcess_CalcTangentSpace;
    if (flags & ModelLoaderFlags::JoinIdenticalVertices) aiFlags |= aiProcess_JoinIdenticalVertices;
    if (flags & ModelLoaderFlags::OptimizeMeshes)        aiFlags |= (aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);
    return aiFlags;
}

static glm::mat4 ConvertMatrix(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static std::shared_ptr<Texture> LoadMaterialTexture(
    const aiScene* scene,
    const aiMaterial* mat,
    aiTextureType type,
    const std::string& directory,
    std::unordered_map<std::string, std::shared_ptr<Texture>>& cache
) {
    aiString str;
    if (mat->GetTexture(type, 0, &str) != AI_SUCCESS || str.length == 0) {
        return nullptr;
    }

    std::string pathStr = str.C_Str();

    auto it = cache.find(pathStr);
    if (it != cache.end()) {
        return it->second;
    }

    // 1. Check embedded textures
    const aiTexture* embedded = nullptr;
    if (pathStr.rfind('*', 0) == 0 && pathStr.length() > 1) {
        int index = std::atoi(pathStr.c_str() + 1);
        if (index >= 0 && index < static_cast<int>(scene->mNumTextures)) {
            embedded = scene->mTextures[index];
        }
    } else {
        embedded = scene->GetEmbeddedTexture(pathStr.c_str());
    }

    if (embedded) {
        std::shared_ptr<Texture> texture = nullptr;
        if (embedded->mHeight == 0) {
            // Compressed buffer in memory (PNG, JPEG, etc.)
            int w = 0, h = 0, channels = 0;
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(embedded->pcData);
            unsigned char* data = stbi_load_from_memory(bytes, static_cast<int>(embedded->mWidth), &w, &h, &channels, 0);
            if (data) {
                texture = Texture::CreateFromPixels(data, w, h, channels);
                stbi_image_free(data);
            }
        } else {
            // Raw uncompressed ARGB8888 texels
            const int totalTexels = static_cast<int>(embedded->mWidth * embedded->mHeight);
            std::vector<unsigned char> rgba(totalTexels * 4);
            for (int i = 0; i < totalTexels; ++i) {
                const auto& texel = embedded->pcData[i];
                rgba[i * 4 + 0] = texel.r;
                rgba[i * 4 + 1] = texel.g;
                rgba[i * 4 + 2] = texel.b;
                rgba[i * 4 + 3] = texel.a;
            }
            texture = Texture::CreateFromPixels(rgba.data(), embedded->mWidth, embedded->mHeight, 4);
        }

        if (texture) {
            cache[pathStr] = texture;
            return texture;
        }
    }

    // 2. Check external file on disk
    std::string filename = GetFilename(pathStr);
    std::vector<std::string> candidatePaths = {
        directory + pathStr,
        directory + filename,
        directory + "textures/" + filename,
        directory + "Textures/" + filename,
        pathStr
    };

    for (auto& p : candidatePaths) {
        std::replace(p.begin(), p.end(), '\\', '/');
        if (FileReader::Exists(p)) {
            auto tex = std::make_shared<Texture>(p);
            if (tex->IsValid()) {
                cache[pathStr] = tex;
                return tex;
            }
        }
    }

    std::cerr << "WARNING::MODEL_LOADER: Could not find texture '" << pathStr << "' in '" << directory << "'" << std::endl;
    return nullptr;
}

static std::shared_ptr<DefaultMaterial> LoadAssimpMaterial(
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

static std::shared_ptr<Mesh> LoadAssimpMesh(const aiMesh* aiM) {
    auto mesh = std::make_shared<Mesh>();
    mesh->vertices.reserve(aiM->mNumVertices);
    mesh->normals.reserve(aiM->mNumVertices);
    mesh->uvs.reserve(aiM->mNumVertices);
    mesh->colors.reserve(aiM->mNumVertices);
    mesh->tangents.reserve(aiM->mNumVertices);

    const bool hasNormals = aiM->HasNormals();
    const bool hasUVs = aiM->HasTextureCoords(0);
    const bool hasColors = aiM->HasVertexColors(0);
    const bool hasTangents = aiM->HasTangentsAndBitangents();

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

        if (hasColors) {
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

static void ProcessNode(
    const aiNode* node,
    const glm::mat4& parentTransform,
    ModelData& model,
    const aiScene* scene
) {
    glm::mat4 localTransform = ConvertMatrix(node->mTransformation);
    glm::mat4 currentTransform = parentTransform * localTransform;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        unsigned int meshIdx = node->mMeshes[i];
        if (meshIdx < model.meshes.size()) {
            ModelPart part;
            part.name = node->mName.C_Str();
            if (part.name.empty()) {
                part.name = "Part_" + std::to_string(model.parts.size());
            }
            part.mesh = model.meshes[meshIdx];
            part.meshIndex = static_cast<int>(meshIdx);
            part.transform = currentTransform;

            unsigned int matIdx = scene->mMeshes[meshIdx]->mMaterialIndex;
            if (matIdx < model.materials.size()) {
                part.material = model.materials[matIdx];
                part.materialIndex = static_cast<int>(matIdx);
            }

            model.parts.push_back(std::move(part));
        }
    }

    for (unsigned int c = 0; c < node->mNumChildren; ++c) {
        ProcessNode(node->mChildren[c], currentTransform, model, scene);
    }
}

} // anonymous namespace

ModelData ModelLoader::Load(const std::string& filepath, ModelLoaderFlags flags) {
    ModelData model;
    Load(filepath, model, flags);
    return model;
}

bool ModelLoader::Load(const std::string& filepath, ModelData& outModel, ModelLoaderFlags flags) {
    outModel = ModelData();
    outModel.filepath = filepath;
    outModel.directory = GetDirectory(filepath);
    outModel.name = GetFilename(filepath);

    Assimp::Importer importer;
    unsigned int aiFlags = ConvertFlags(flags);

    const aiScene* scene = importer.ReadFile(filepath, aiFlags);
    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        std::cerr << "ERROR::MODEL_LOADER::ASSIMP_IMPORT_FAILED: " << importer.GetErrorString()
                  << " (" << filepath << ")" << std::endl;
        return false;
    }

    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;

    // 1. Load all materials
    outModel.materials.reserve(scene->mNumMaterials);
    for (unsigned int m = 0; m < scene->mNumMaterials; ++m) {
        outModel.materials.push_back(LoadAssimpMaterial(scene, scene->mMaterials[m], outModel.directory, textureCache));
    }

    // 2. Load all meshes
    outModel.meshes.reserve(scene->mNumMeshes);
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        outModel.meshes.push_back(LoadAssimpMesh(scene->mMeshes[m]));
    }

    // 3. Process node hierarchy into parts
    if (scene->mRootNode) {
        ProcessNode(scene->mRootNode, glm::mat4(1.0f), outModel, scene);
    }

    // Fallback if no parts were linked by nodes
    if (outModel.parts.empty()) {
        for (size_t i = 0; i < outModel.meshes.size(); ++i) {
            ModelPart part;
            part.name = "Mesh_" + std::to_string(i);
            part.mesh = outModel.meshes[i];
            part.meshIndex = static_cast<int>(i);
            unsigned int matIdx = scene->mMeshes[i]->mMaterialIndex;
            if (matIdx < outModel.materials.size()) {
                part.material = outModel.materials[matIdx];
                part.materialIndex = static_cast<int>(matIdx);
            }
            part.transform = glm::mat4(1.0f);
            outModel.parts.push_back(std::move(part));
        }
    }

    // 4. Collect all loaded unique textures
    for (auto& [path, tex] : textureCache) {
        if (tex) {
            outModel.textures.push_back(tex);
        }
    }

    return outModel.IsValid();
}

std::vector<std::shared_ptr<Mesh>> ModelLoader::LoadMeshes(const std::string& filepath, ModelLoaderFlags flags) {
    ModelData model = Load(filepath, flags);
    return model.meshes;
}

std::vector<std::shared_ptr<Texture>> ModelLoader::LoadTextures(const std::string& filepath) {
    ModelData model = Load(filepath);
    return model.textures;
}

std::vector<std::shared_ptr<DefaultMaterial>> ModelLoader::LoadMaterials(const std::string& filepath) {
    ModelData model = Load(filepath);
    return model.materials;
}
