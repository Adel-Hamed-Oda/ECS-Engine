#pragma once

#include <memory>
#include <string>
#include <utility>
#include <glm/glm.hpp>

#include "ScreenspaceMaterial.h"

class TextMaterial : public ScreenspaceMaterial {
public:
    glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 1.0f);

    explicit TextMaterial(std::shared_ptr<Shader> shader = nullptr, const glm::vec3& color = glm::vec3(1.0f))
        : ScreenspaceMaterial(std::move(shader)), textColor(color)
    {
        if (!this->shader)
        {
            this->shader = GetDefaultShader();
        }
    }

    ~TextMaterial() override = default;

    void ApplyMaterial(
        const Transform2D* transform2D,
        const glm::mat4& orthoProjection
    ) override
    {
        if (!SetupScreenSpaceUniforms(transform2D, orthoProjection)) return;

        shader->setVec3("textColor", textColor);
        shader->setInt("text", 0);
    }

    static std::shared_ptr<Shader> GetDefaultShader()
    {
        if (defaultShader && defaultShader->IsValid())
        {
            return defaultShader;
        }

        const std::string textVertexShader = R"(
            #version 330 core
            layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
            out vec2 TexCoords;

            uniform mat4 projection;
            uniform mat4 view;
            uniform mat4 model;

            void main()
            {
                gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
                TexCoords = vertex.zw;
            }
        )";

        const std::string textFragmentShader = R"(
            #version 330 core
            in vec2 TexCoords;
            out vec4 FragColor;

            uniform sampler2D text;
            uniform vec3 textColor;

            void main()
            {
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
                FragColor = vec4(textColor, 1.0) * sampled;
            }
        )";

        defaultShader = Shader::CreateFromSource(textVertexShader, textFragmentShader);
        return defaultShader;
    }

private:
    inline static std::shared_ptr<Shader> defaultShader = nullptr;
};
