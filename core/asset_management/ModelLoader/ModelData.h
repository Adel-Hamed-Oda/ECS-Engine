#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/rendering/mesh/Mesh.h"
#include "core/rendering/texture/Texture.h"
#include "core/rendering/material/DefaultMaterial.h"

struct ModelPart {
    std::string name;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<DefaultMaterial> material;
    glm::mat4 transform = glm::mat4(1.0f);
    int meshIndex = -1;
    int materialIndex = -1;
};

struct ModelData {
    std::string name;
    std::string filepath;
    std::string directory;

    // Extracted assets
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<std::shared_ptr<DefaultMaterial>> materials;

    // Hierarchical model parts (mesh + material + transform)
    std::vector<ModelPart> parts;

    bool IsValid() const {
        return !meshes.empty();
    }

    size_t GetMeshCount() const { return meshes.size(); }
    size_t GetTextureCount() const { return textures.size(); }
    size_t GetMaterialCount() const { return materials.size(); }
    size_t GetPartCount() const { return parts.size(); }
};