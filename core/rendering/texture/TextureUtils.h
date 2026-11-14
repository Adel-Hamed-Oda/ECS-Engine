#pragma once

#include "core/private_imports.h"
#include "Texture.h"

class TextureUtils
{
public:
    static glm::vec2 GetImageDimensions(const std::string& imagePath);
    static Texture* CreateFromColor(glm::vec4 color, glm::vec2 size);
};

// Convenience free function matching CreateFromColor signature
inline Texture* CreateFromColor(glm::vec4 color, glm::vec2 size)
{
    return TextureUtils::CreateFromColor(color, size);
}
