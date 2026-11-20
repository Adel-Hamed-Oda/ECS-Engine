#pragma once

#include "core/private_imports.h"
#include "core/rendering/buffers/VAO.h"
#include "core/rendering/buffers/VBO.h"
#include "core/rendering/texture/Texture.h"
#include "core/rendering/shader/Shader.h"
#include "core/inbuilt/components/Transform2D.h"
#include "core/rendering/material/ImageMaterial.h"

struct ImageRenderer {
    friend class Renderer;

public:
    std::shared_ptr<Texture> texture = nullptr;
    std::shared_ptr<ImageMaterial> material = nullptr;
    glm::vec4 color = glm::vec4(1.0f);

    ImageRenderer(
        std::shared_ptr<Texture> texture = nullptr,
        std::shared_ptr<ImageMaterial> material = nullptr,
        const glm::vec4& color = glm::vec4(1.0f)
    )
        : texture(std::move(texture)),
          material(std::move(material)),
          color(color)
    {
        if (!this->material)
        {
            this->material = std::make_shared<ImageMaterial>(nullptr, this->texture, this->color);
        }
        InitializeBuffers();
    }

    ImageRenderer(const ImageRenderer&) = delete;
    ImageRenderer& operator=(const ImageRenderer&) = delete;

    ImageRenderer(ImageRenderer&& other) noexcept
        : texture(std::move(other.texture)),
          material(std::move(other.material)),
          color(other.color),
          vao(std::move(other.vao)),
          vbo(std::move(other.vbo))
    {
    }

    ImageRenderer& operator=(ImageRenderer&& other) noexcept
    {
        if (this != &other)
        {
            Cleanup();

            texture = std::move(other.texture);
            material = std::move(other.material);
            color = other.color;
            vao = std::move(other.vao);
            vbo = std::move(other.vbo);
        }
        return *this;
    }

    ~ImageRenderer() { Cleanup(); }

    void Draw(
        const Transform2D* transform2D,
        const glm::mat4& orthoProjection,
        const glm::vec4& tint = glm::vec4(1.0f)
    ) const
    {
        if (!vao.IsValid() || !vbo.IsValid())
            return;

        ImageMaterial* mat = material.get();
        if (mat)
        {
            if (texture && texture->IsValid())
            {
                mat->texture = texture;
            }
            mat->color = color * tint;
            mat->ApplyMaterial(transform2D, orthoProjection);
        }

        vao.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        VAO::Unbind();

        if (texture && texture->IsValid())
        {
            Texture::Unbind(0);
        }
    }

private:
    VAO vao;
    VBO vbo;

    void Cleanup()
    {
        vbo.Delete();
        vao.Delete();
    }

    void InitializeBuffers()
    {
        if (!vao.IsValid()) vao.Create();
        if (!vao.IsValid()) return;

        // Standard unit quad: (0,0) to (1,1) with matching UVs
        const float quadVertices[6 * 4] = {
            // Pos (x, y)    UV (u, v)
            0.0f, 0.0f,      0.0f, 0.0f,
            1.0f, 0.0f,      1.0f, 0.0f,
            1.0f, 1.0f,      1.0f, 1.0f,

            0.0f, 0.0f,      0.0f, 0.0f,
            1.0f, 1.0f,      1.0f, 1.0f,
            0.0f, 1.0f,      0.0f, 1.0f
        };

        vbo = VBO(sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        vao.LinkAttrib(vbo, 0, 4, GL_FLOAT, 4 * sizeof(float), nullptr);

        VAO::Unbind();
        VBO::Unbind();
    }
};
