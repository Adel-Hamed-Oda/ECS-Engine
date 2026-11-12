#pragma once

#include "core/private_imports.h"
#include "core/rendering/buffers/VAO.h"
#include "core/rendering/buffers/VBO.h"
#include "core/rendering/font/Font.h"
#include "core/rendering/shader/Shader.h"

struct TextRenderer {
    friend class Renderer;

public:
    std::string text = "";
    std::shared_ptr<Font> font;
    std::shared_ptr<Shader> shader;

    TextRenderer(
        std::string text = "",
        std::shared_ptr<Font> font = nullptr,
        std::shared_ptr<Shader> shader = nullptr
    )
        : text(std::move(text)), font(std::move(font)), shader(std::move(shader))
    {
        InitializeBuffers();
    }

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    TextRenderer(TextRenderer&& other) noexcept
        : text(std::move(other.text)),
          font(std::move(other.font)),
          shader(std::move(other.shader)),
          vao(std::move(other.vao)),
          positionVBO(std::move(other.positionVBO)),
          normalVBO(std::move(other.normalVBO)),
          colorVBO(std::move(other.colorVBO)),
          uvVBO(std::move(other.uvVBO))
    {
    }

    TextRenderer& operator=(TextRenderer&& other) noexcept
    {
        if (this != &other)
        {
            Cleanup();

            text = std::move(other.text);
            font = std::move(other.font);
            shader = std::move(other.shader);
            vao = std::move(other.vao);
            positionVBO = std::move(other.positionVBO);
            normalVBO = std::move(other.normalVBO);
            colorVBO = std::move(other.colorVBO);
            uvVBO = std::move(other.uvVBO);
        }
        return *this;
    }

    ~TextRenderer() { Cleanup(); }

    void Draw(float x, float y, glm::vec2 scale = glm::vec2(1.0f), const glm::vec3& color = glm::vec3(1.0f)) const {
        if (!vao.IsValid() || !positionVBO.IsValid() || !font || !font->IsValid() || text.empty())
            return;

        // activate corresponding render state
        if (shader && shader->IsValid())
        {
            shader->use();
            shader->setVec3("textColor", color);
        }
        glActiveTexture(GL_TEXTURE0);
        vao.Bind();
        positionVBO.Bind();

        // iterate through all characters
        for (const char c : text)
        {
            const Font::Character* ch = font->GetCharacter(c);
            if (!ch)
                continue;

            const float w = static_cast<float>(ch->Size.x) * scale.x;
            const float h = static_cast<float>(ch->Size.y) * scale.y;

            if (ch->TextureID != 0 && w > 0.0f && h > 0.0f)
            {
                const float xpos = x + static_cast<float>(ch->Bearing.x) * scale.x;
                const float ypos = y - static_cast<float>(ch->Size.y - ch->Bearing.y) * scale.y;

                // update VBO for each character
                const float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }
                };

                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch->TextureID);
                // update content of VBO memory
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                // render quad
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            x += static_cast<float>(ch->Advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64)
        }

        VBO::Unbind();
        VAO::Unbind();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    VAO vao;
    VBO positionVBO;
    VBO normalVBO;
    VBO colorVBO;
    VBO uvVBO;

    void Cleanup()
    {
        positionVBO.Delete();
        normalVBO.Delete();
        colorVBO.Delete();
        uvVBO.Delete();
        vao.Delete();
    }

    void InitializeBuffers()
    {
        if (!vao.IsValid()) vao.Create();
        if (!vao.IsValid()) return;

        positionVBO = VBO(sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        vao.LinkAttrib(positionVBO, 0, 4, GL_FLOAT, 4 * sizeof(float), nullptr);

        VAO::Unbind();
        VBO::Unbind();
    }
};