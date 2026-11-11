#pragma once

#include "core/private_imports.h"

class VBO {
public:
    GLuint ID = 0;

    VBO() = default;

    VBO(const GLsizeiptr sizeInBytes, const void* data, const GLenum usage = GL_STATIC_DRAW) {
        if (sizeInBytes <= 0 || !glad_glGenBuffers) return;

        glGenBuffers(1, &ID);
        if (ID == 0) {
            std::cerr << "ERROR::VBO::CREATION_FAILED" << std::endl;
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    template <typename T>
    explicit VBO(const std::vector<T>& data, const GLenum usage = GL_STATIC_DRAW)
        : VBO(static_cast<GLsizeiptr>(data.size() * sizeof(T)), data.data(), usage) {}

    VBO(const VBO&) = delete;
    VBO& operator=(const VBO&) = delete;

    VBO(VBO&& other) noexcept : ID(other.ID) {
        other.ID = 0;
    }

    VBO& operator=(VBO&& other) noexcept {
        if (this != &other) {
            Delete();
            ID = other.ID;
            other.ID = 0;
        }
        return *this;
    }

    ~VBO() {
        Delete();
    }

    bool IsValid() const { return ID != 0; }
    void Bind() const { if (ID != 0) glBindBuffer(GL_ARRAY_BUFFER, ID); }
    static void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    void Delete() {
        if (ID != 0) {
            glDeleteBuffers(1, &ID);
            ID = 0;
        }
    }
};
