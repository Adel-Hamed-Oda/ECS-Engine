#pragma once

#include <glad/gl.h>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <glm/glm.hpp>

#include "TextureEnums.h"

class Texture
{
public:
    unsigned int ID = 0;

    TextureWrapMode wrapMode = TextureWrapMode::REPEAT;
    TextureFilterMode filterMode = TextureFilterMode::LINEAR;
    TextureLoadMode loadMode = TextureLoadMode::NORMAL;

    Texture() = default;

    // Single primary constructor for loading a texture from a file path
    explicit Texture(
        const std::string& texturePath,
        TextureLoadMode loadMode = TextureLoadMode::NORMAL,
        TextureWrapMode wrap = TextureWrapMode::REPEAT,
        TextureFilterMode filter = TextureFilterMode::LINEAR
    );

    // Constructor for creating a texture from raw memory pixels
    Texture(
        const unsigned char* data,
        int width,
        int height,
        int nrChannels,
        TextureLoadMode loadMode = TextureLoadMode::NORMAL,
        TextureWrapMode wrap = TextureWrapMode::REPEAT,
        TextureFilterMode filter = TextureFilterMode::LINEAR
    );

    ~Texture();

    Texture(const Texture& other);
    Texture& operator=(const Texture& other);

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    Texture* Copy() const;

    // Binds this texture to the specified texture unit (default GL_TEXTURE0)
    void Bind(unsigned int slot = 0) const;
    static void Unbind(unsigned int slot = 0);

    bool IsValid() const;

    const std::string& GetPath() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetChannels() const;
    const std::vector<unsigned char>& GetRawPixels() const;

    static std::vector<Texture*> CreateTextureMap(
        const std::string& path,
        int columns,
        int rows,
        TextureLoadMode loadMode = TextureLoadMode::NORMAL,
        TextureWrapMode wrap = TextureWrapMode::REPEAT,
        TextureFilterMode filter = TextureFilterMode::LINEAR
    );

    static glm::vec2 GetImageDimensions(const std::string& imagePath);
    static Texture* CreateFromColor(glm::vec4 color, glm::vec2 size);

    // Updates existing GPU texture with new pixel data without recreating GL texture ID (if dimensions match)
    void UpdatePixels(const unsigned char* data, int width, int height, int nrChannels = 4);

    // Static helper to create a shared_ptr<Texture> from raw memory pixels
    static std::shared_ptr<Texture> CreateFromPixels(
        const unsigned char* data,
        int width,
        int height,
        int nrChannels = 4,
        TextureWrapMode wrap = TextureWrapMode::CLAMP_TO_EDGE,
        TextureFilterMode filter = TextureFilterMode::NEAREST
    );
    
private:
    std::string path;
    int width = 0;
    int height = 0;
    int nrChannels = 0;
    std::vector<unsigned char> rawPixels;

    void LoadFromFile(const std::string& texturePath);
    void LoadFromPixels(const unsigned char* data, int w, int h, int channels);
    void UploadToGPU(const unsigned char* data, int width, int height, int nrChannels);
    static GLenum GetFormatFromChannels(int nrChannels);
};
