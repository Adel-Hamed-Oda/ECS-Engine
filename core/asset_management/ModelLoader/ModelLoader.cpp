#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <unordered_map>

namespace {

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

std::string ModelLoader::GetDirectory(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(0, lastSlash + 1);
    }
    return "";
}

std::string ModelLoader::GetFilename(const std::string& filepath) {
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return filepath.substr(lastSlash + 1);
    }
    return filepath;
}

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

    // 2. Load all meshes (and extract bone weights into skeleton)
    outModel.skeleton = std::make_shared<Skeleton>();
    outModel.meshes.reserve(scene->mNumMeshes);
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        outModel.meshes.push_back(LoadAssimpMesh(scene->mMeshes[m], outModel.skeleton.get()));
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

    // 4. Build skeleton node hierarchy and load animation clips
    if (outModel.skeleton->HasBones() || scene->HasAnimations()) {
        LoadAssimpAnimations(scene, outModel);
    }

    // 5. Collect all loaded unique textures
    for (auto& [path, tex] : textureCache) {
        if (tex) {
            outModel.textures.push_back(tex);
        }
    }

    return outModel.IsValid();
}
