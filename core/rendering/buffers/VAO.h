#pragma once

#include "core/private_imports.h"
#include "core/rendering/buffers/VBO.h"
#include "core/rendering/buffers/EBO.h"

class VAO {
public:
    GLuint ID = 0;

    VAO() {
        Create();
    }

    VAO(const VAO&) = delete;
    VAO& operator=(const VAO&) = delete;

    VAO(VAO&& other) noexcept : ID(other.ID) {
        other.ID = 0;
    }

    VAO& operator=(VAO&& other) noexcept {
        if (this != &other) {
            Delete();
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

    ~VAO() {
        Delete();
    }

    void Create() {
        if (ID != 0 || !glad_glGenVertexArrays) return;

        glGenVertexArrays(1, &ID);
        if (ID == 0) {
            std::cerr << "ERROR::VAO::CREATION_FAILED" << std::endl;
        }
    }

    void Delete() {
        if (ID != 0) {
            glDeleteVertexArrays(1, &ID);
            ID = 0;
        }
    }

    bool IsValid() const { return ID != 0; }
    void Bind() const { if (ID != 0) glBindVertexArray(ID); }
    static void Unbind() { glBindVertexArray(0); }

    void LinkAttrib(
        const VBO& vbo,
        const GLuint layout,
        const GLint numComponents,
        const GLenum type,
        const GLsizei stride,
        const void* offset = nullptr,
        const GLboolean normalized = GL_FALSE
    ) const {
        if (ID == 0 || !vbo.IsValid()) return;

        Bind();
        vbo.Bind();
        glVertexAttribPointer(layout, numComponents, type, normalized, stride, offset);
        glEnableVertexAttribArray(layout);
        VBO::Unbind();
    }

    void LinkEBO(const EBO& ebo) const {
        if (ID == 0 || !ebo.IsValid()) return;

        Bind();
        ebo.Bind();
    }
};
