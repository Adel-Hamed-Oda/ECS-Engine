#pragma once

#include "core/private_imports.h"

class EBO {
public:
    GLuint ID = 0;

    EBO() = default;

    EBO(const size_t count, const void* data, const GLenum usage = GL_STATIC_DRAW)
        : count(count)
    {
        if (count == 0 || !glad_glGenBuffers) {
            this->count = 0;
            return;
        }

        glGenBuffers(1, &ID);
        if (ID == 0) {
            std::cerr << "ERROR::EBO::CREATION_FAILED" << std::endl;
            this->count = 0;
            return;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(count * sizeof(unsigned int)), data, usage);
    }

    explicit EBO(const std::vector<unsigned int>& indices, const GLenum usage = GL_STATIC_DRAW)
        : EBO(indices.size(), indices.data(), usage) {}

    EBO(const EBO&) = delete;
    EBO& operator=(const EBO&) = delete;

    EBO(EBO&& other) noexcept
        : ID(other.ID), count(other.count)
    {
        other.ID = 0;
        other.count = 0;
    }

    EBO& operator=(EBO&& other) noexcept {
        if (this != &other) {
            Delete();
            ID = other.ID;
            count = other.count;
            other.ID = 0;
            other.count = 0;
        }
        return *this;
    }

    ~EBO() {
        Delete();
    }

    bool IsValid() const { return ID != 0; }
    size_t GetCount() const { return count; }
    void Bind() const { if (ID != 0) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
    static void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    void Delete() {
        if (ID != 0) {
            glDeleteBuffers(1, &ID);
            ID = 0;
        }
        count = 0;
    }

private:
    size_t count = 0;
};
