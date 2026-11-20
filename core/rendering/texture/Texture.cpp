#include "Texture.h"

Texture::Texture(
    const std::string& texturePath,
    TextureLoadMode loadMode,
    TextureWrapMode wrap,
    TextureFilterMode filter
)
    : wrapMode(wrap),
      filterMode(filter),
      loadMode(loadMode)
{
    LoadFromFile(texturePath);
}

Texture::Texture(
    const unsigned char* data,
    int width,
    int height,
    int nrChannels,
    TextureLoadMode loadMode,
    TextureWrapMode wrap,
    TextureFilterMode filter
)
    : wrapMode(wrap),
      filterMode(filter),
      loadMode(loadMode)
{
    LoadFromPixels(data, width, height, nrChannels);
}

Texture::~Texture()
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
        ID = 0;
    }
}

Texture::Texture(const Texture& other)
    : wrapMode(other.wrapMode),
      filterMode(other.filterMode),
      loadMode(other.loadMode),
      path(other.path),
      width(other.width),
      height(other.height),
      nrChannels(other.nrChannels),
      rawPixels(other.rawPixels)
{
    if (!rawPixels.empty())
    {
        LoadFromPixels(rawPixels.data(), width, height, nrChannels);
    }
    else if (!path.empty())
    {
        LoadFromFile(path);
    }
}

Texture& Texture::operator=(const Texture& other)
{
    if (this != &other)
    {
        if (ID != 0)
        {
            glDeleteTextures(1, &ID);
            ID = 0;
        }

        wrapMode = other.wrapMode;
        filterMode = other.filterMode;
        loadMode = other.loadMode;
        path = other.path;
        width = other.width;
        height = other.height;
        nrChannels = other.nrChannels;
        rawPixels = other.rawPixels;

        if (!rawPixels.empty())
        {
            LoadFromPixels(rawPixels.data(), width, height, nrChannels);
        }
        else if (!path.empty())
        {
            LoadFromFile(path);
        }
    }
    return *this;
}

Texture::Texture(Texture&& other) noexcept
    : ID(std::exchange(other.ID, 0)),
      wrapMode(other.wrapMode),
      filterMode(other.filterMode),
      loadMode(other.loadMode),
      path(std::move(other.path)),
      width(other.width),
      height(other.height),
      nrChannels(other.nrChannels),
      rawPixels(std::move(other.rawPixels))
{
    other.width = 0;
    other.height = 0;
    other.nrChannels = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        if (ID != 0)
        {
            glDeleteTextures(1, &ID);
        }

        ID = std::exchange(other.ID, 0);
        wrapMode = other.wrapMode;
        filterMode = other.filterMode;
        loadMode = other.loadMode;
        path = std::move(other.path);
        width = other.width;
        height = other.height;
        nrChannels = other.nrChannels;
        rawPixels = std::move(other.rawPixels);

        other.width = 0;
        other.height = 0;
        other.nrChannels = 0;
    }
    return *this;
}

Texture* Texture::Copy() const
{
    return new Texture(*this);
}

void Texture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind(unsigned int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::IsValid() const
{
    return ID != 0;
}

const std::string& Texture::GetPath() const
{
    return path;
}

int Texture::GetWidth() const
{
    return width;
}

int Texture::GetHeight() const
{
    return height;
}

int Texture::GetChannels() const
{
    return nrChannels;
}

const std::vector<unsigned char>& Texture::GetRawPixels() const
{
    return rawPixels;
}

void Texture::LoadFromFile(const std::string& texturePath)
{
    path = texturePath;
    stbi_set_flip_vertically_on_load(true);
    int w = 0, h = 0, channels = 0;
    unsigned char* data = stbi_load(texturePath.c_str(), &w, &h, &channels, 0);

    if (!data)
    {
        std::cerr << "ERROR::TEXTURE::FILE_NOT_FOUND_OR_READ_FAILED: " << texturePath << std::endl;
        stbi_set_flip_vertically_on_load(false);
        return;
    }

    width = w;
    height = h;
    nrChannels = channels;
    rawPixels.assign(data, data + (static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(channels)));

    UploadToGPU(data, width, height, nrChannels);

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);
}

void Texture::LoadFromPixels(const unsigned char* data, int w, int h, int channels)
{
    if (!data || w <= 0 || h <= 0)
    {
        std::cerr << "ERROR::TEXTURE::CREATION_FROM_PIXELS_FAILED" << std::endl;
        return;
    }

    width = w;
    height = h;
    nrChannels = channels;
    if (rawPixels.data() != data)
    {
        rawPixels.assign(data, data + (static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(channels)));
    }

    UploadToGPU(data, width, height, nrChannels);
}

void Texture::UploadToGPU(const unsigned char* data, int w, int h, int channels)
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
        ID = 0;
    }

    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrapMode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrapMode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(filterMode));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(filterMode));

    const GLenum format = GetFormatFromChannels(channels);
    GLint previousUnpackAlignment = 4;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
    glGenerateMipmap(GL_TEXTURE_2D);

    ID = texID;
}

GLenum Texture::GetFormatFromChannels(int channels)
{
    switch (channels)
    {
        case 1: return GL_RED;
        case 2: return GL_RG;
        case 3: return GL_RGB;
        case 4: return GL_RGBA;
        default:
            std::cerr << "ERROR::TEXTURE::UNSUPPORTED_CHANNEL_COUNT: "
                      << channels << std::endl;
            return GL_RGB;
    }
}

void Texture::UpdatePixels(const unsigned char* data, int w, int h, int channels)
{
    if (!data || w <= 0 || h <= 0) return;

    if (ID != 0 && width == w && height == h && nrChannels == channels)
    {
        glBindTexture(GL_TEXTURE_2D, ID);
        const GLenum format = GetFormatFromChannels(channels);
        GLint previousUnpackAlignment = 4;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
        glBindTexture(GL_TEXTURE_2D, 0);

        rawPixels.assign(data, data + (static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(channels)));
    }
    else
    {
        LoadFromPixels(data, w, h, channels);
    }
}

std::shared_ptr<Texture> Texture::CreateFromPixels(
    const unsigned char* data,
    int width,
    int height,
    int nrChannels,
    TextureWrapMode wrap,
    TextureFilterMode filter
)
{
    return std::make_shared<Texture>(data, width, height, nrChannels, TextureLoadMode::NORMAL, wrap, filter);
}
