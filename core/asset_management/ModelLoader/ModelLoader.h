#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ModelData.h"
#include "ModelLoadingFlags.h"

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
};
