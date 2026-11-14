#include "Texture.h"

std::vector<Texture*> Texture::CreateTextureMap(
    const std::string& path,
    int columns,
    int rows,
    TextureLoadMode loadMode,
    TextureWrapMode wrap,
    TextureFilterMode filter
)
{
    std::vector<Texture*> textures;

    if (columns <= 0 || rows <= 0)
    {
        std::cerr << "ERROR::TEXTURE::TEXTURE_MAP::INVALID_GRID: " << path
                  << " (columns and rows must be positive)" << std::endl;
        return textures;
    }

    stbi_set_flip_vertically_on_load(false);
    int sheetWidth = 0, sheetHeight = 0, nrChannels = 0;
    unsigned char* sheetData = stbi_load(path.c_str(), &sheetWidth, &sheetHeight, &nrChannels, 0);

    if (!sheetData)
    {
        std::cerr << "ERROR::TEXTURE::TEXTURE_MAP::FILE_NOT_FOUND_OR_READ_FAILED: " << path << std::endl;
        return textures;
    }

    const int tileWidth = sheetWidth / columns;
    const int tileHeight = sheetHeight / rows;

    if (tileWidth <= 0 || tileHeight <= 0)
    {
        std::cerr << "ERROR::TEXTURE::TEXTURE_MAP::TILE_SIZE_TOO_SMALL: " << path << std::endl;
        stbi_image_free(sheetData);
        return textures;
    }

    textures.reserve(static_cast<size_t>(columns * rows));
    const size_t rowBytes = static_cast<size_t>(tileWidth) * static_cast<size_t>(nrChannels);
    std::vector<unsigned char> tileBuffer(rowBytes * static_cast<size_t>(tileHeight));

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < columns; ++c)
        {
            const int startX = c * tileWidth;
            const int startY = r * tileHeight;

            for (int row = 0; row < tileHeight; ++row)
            {
                // Vertical flip within each tile for OpenGL UV orientation
                const int srcY = startY + (tileHeight - 1 - row);
                const size_t srcOffset =
                    (static_cast<size_t>(srcY) * static_cast<size_t>(sheetWidth) + static_cast<size_t>(startX)) *
                    static_cast<size_t>(nrChannels);
                const size_t dstOffset = static_cast<size_t>(row) * rowBytes;

                std::memcpy(tileBuffer.data() + dstOffset, sheetData + srcOffset, rowBytes);
            }

            Texture* tex = new Texture(
                tileBuffer.data(),
                tileWidth,
                tileHeight,
                nrChannels,
                loadMode,
                wrap,
                filter
            );

            if (tex->IsValid())
            {
                textures.push_back(tex);
            }
            else
            {
                delete tex;
            }
        }
    }

    stbi_image_free(sheetData);
    return textures;
}
