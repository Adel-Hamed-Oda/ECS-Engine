#pragma once

#include <memory>
#include <utility>
#include <glm/glm.hpp>

#include "core/FallBacks.h"
#include "core/rendering/material/ScreenspaceMaterial.h"
#include "core/rendering/texture/Texture.h"

class ImageMaterial : public ScreenspaceMaterial {
public:
    std::shared_ptr<Texture> texture = nullptr;
    glm::vec4 color = glm::vec4(1.0f);

    explicit ImageMaterial(
        std::shared_ptr<Shader> shader = nullptr,
        std::shared_ptr<Texture> texture = nullptr,
        const glm::vec4& color = glm::vec4(1.0f)
    )
        : ScreenspaceMaterial(std::move(shader)),
          texture(std::move(texture)),
          color(color)
    {
        if (!this->shader)
        {
            this->shader = GetDefaultShader();
        }
    }

    ~ImageMaterial() override = default;

    void ApplyMaterial(
        const Transform2D* transform2D,
        const glm::mat4& viewProj
    ) override
    {
        if (!SetupScreenSpaceUniforms(transform2D, viewProj)) return;

        shader->setVec4("tintColor", color);
        shader->setInt("imageTexture", 0);

        bool validTex = (texture && texture->IsValid());
        shader->setBool("hasTexture", validTex);

        if (validTex)
        {
            texture->Bind(0);
        }
    }

    static std::shared_ptr<Shader> GetDefaultShader()
    {
        if (defaultShader && defaultShader->IsValid())
        {
            return defaultShader;
        }

        defaultShader = std::make_shared<Shader>(
            FallBacks::ImageVertexShaderCode,
            FallBacks::ImageFragmentShaderCode
        );
        return defaultShader;
    }

private:
    inline static std::shared_ptr<Shader> defaultShader = nullptr;
};
