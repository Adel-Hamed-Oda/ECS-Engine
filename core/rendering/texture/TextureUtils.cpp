#include "Texture.h"
#include <stb_image.h>
#include <algorithm>
#include <vector>

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

// min and max are used to ensure that the width and height are at least 1 pixel
Texture* Texture::CreateFromColor(glm::vec4 color, glm::vec2 size)
{
    int width = std::max(1, static_cast<int>(size.x));
    int height = std::max(1, static_cast<int>(size.y));

    unsigned char r = static_cast<unsigned char>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
    unsigned char g = static_cast<unsigned char>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
    unsigned char b = static_cast<unsigned char>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
    unsigned char a = static_cast<unsigned char>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);

    const size_t totalPixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    std::vector<unsigned char> pixels(totalPixels * 4);

    for (size_t i = 0; i < totalPixels; ++i)
    {
        pixels[i * 4 + 0] = r;
        pixels[i * 4 + 1] = g;
        pixels[i * 4 + 2] = b;
        pixels[i * 4 + 3] = a;
    }

    return new Texture(pixels.data(), width, height, 4);
}
