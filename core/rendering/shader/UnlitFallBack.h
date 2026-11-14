#pragma once

#include "core/private_imports.h"

class UnlitFallBack {
public:
    inline static const std::string VertexShaderCode = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec4 aColor;
        layout (location = 3) in vec2 aUV;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec4 FragVertexColor;
        out vec2 FragTextureCoordinate;

        void main()
        {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            FragVertexColor = aColor;
            FragTextureCoordinate = aUV;
        }
    )";

    inline static const std::string FragmentShaderCode = R"(
        #version 330 core
        in vec4 FragVertexColor;
        in vec2 FragTextureCoordinate;

        struct Material {
            bool hasTexture;
            sampler2D textureSampler;
            vec4 baseColor;
        };
        uniform Material material;

        uniform bool hasTexture;
        uniform sampler2D textureSampler;
        uniform vec4 color;

        out vec4 FragColor;

        void main()
        {
            vec4 base = (material.baseColor != vec4(0.0)) ? material.baseColor : ((color != vec4(0.0)) ? color : vec4(1.0));
            vec4 result = FragVertexColor * base;

            if (material.hasTexture)
            {
                result *= texture(material.textureSampler, FragTextureCoordinate);
            }
            else if (hasTexture)
            {
                result *= texture(textureSampler, FragTextureCoordinate);
            }

            FragColor = result;
        }
    )";
};

using UnlitFallBacks = UnlitFallBack;
