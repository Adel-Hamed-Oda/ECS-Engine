#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "ModelData.h"
#include "ModelLoadingFlags.h"

struct aiScene;
struct aiMaterial;
struct aiMesh;

class ModelLoader {
public:
    // Core loader: loads a 3D model/scene file and extracts meshes, textures, materials, and parts
    static ModelData Load(const std::string& filepath, ModelLoaderFlags flags = ModelLoaderFlags::Default);
    static bool Load(const std::string& filepath, ModelData& outModel, ModelLoaderFlags flags = ModelLoaderFlags::Default);

    // Convenience extractors for specific assets
    static std::vector<std::shared_ptr<Mesh>> LoadMeshes(const std::string& filepath, ModelLoaderFlags flags = ModelLoaderFlags::Default);
    static std::vector<std::shared_ptr<Texture>> LoadTextures(const std::string& filepath);
    static std::vector<std::shared_ptr<DefaultMaterial>> LoadMaterials(const std::string& filepath);

private:
    ModelLoader() = delete;

    static std::string GetDirectory(const std::string& filepath);
    static std::string GetFilename(const std::string& filepath);

    static std::shared_ptr<Mesh> LoadAssimpMesh(const aiMesh* aiM);

    static std::shared_ptr<DefaultMaterial> LoadAssimpMaterial(
        const aiScene* scene,
        const aiMaterial* aiMat,
        const std::string& directory,
        std::unordered_map<std::string, std::shared_ptr<Texture>>& cache
    );

    static std::shared_ptr<Texture> LoadMaterialTexture(
        const aiScene* scene,
        const aiMaterial* mat,
        int type,
        const std::string& directory,
        std::unordered_map<std::string, std::shared_ptr<Texture>>& cache
    );
};
