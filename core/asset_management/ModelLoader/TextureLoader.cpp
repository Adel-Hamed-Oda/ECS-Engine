#include "ModelLoader.h"

#include <assimp/scene.h>
#include <assimp/material.h>
#include <stb_image.h>

#include <iostream>
#include <algorithm>
#include <cstdlib>

#include "core/asset_management/FileReader.h"

std::shared_ptr<Texture> ModelLoader::LoadMaterialTexture(
    const aiScene* scene,
    const aiMaterial* mat,
    int type,
    const std::string& directory,
    std::unordered_map<std::string, std::shared_ptr<Texture>>& cache
) {
    aiString str;
    if (mat->GetTexture(static_cast<aiTextureType>(type), 0, &str) != AI_SUCCESS || str.length == 0) {
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
        directory + "../textures/" + filename,
        directory + "../Textures/" + filename,
        directory + "../" + filename,
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

std::vector<std::shared_ptr<Texture>> ModelLoader::LoadTextures(const std::string& filepath) {
    ModelData model = Load(filepath);
    return model.textures;
}
