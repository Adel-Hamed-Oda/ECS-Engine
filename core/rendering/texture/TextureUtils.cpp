#include "Texture.h"

glm::vec2 Texture::GetImageDimensions(const std::string& imagePath)
{
    int channels = 0;
    int width = 0, height = 0;
    if (stbi_info(imagePath.c_str(), &width, &height, &channels))
    {
        return glm::vec2(static_cast<float>(width), static_cast<float>(height));
    }
    return glm::vec2(0.0f);
}
