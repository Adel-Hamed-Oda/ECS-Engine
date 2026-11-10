#pragma once

#include "core/private_imports.h"
#include "core/rendering/mesh/Mesh.h"
#include "core/rendering/shader/Shader.h"

struct MeshRenderer
{
public:
    std::unique_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;

    MeshRenderer(std::unique_ptr<Mesh> mesh, std::shared_ptr<Shader> shader = nullptr)
        : mesh(std::move(mesh)), shader(std::move(shader))
    {
        InitializeBuffers();
    }

    MeshRenderer(const MeshRenderer&) = delete;
    MeshRenderer& operator=(const MeshRenderer&) = delete;

    MeshRenderer(MeshRenderer&& other) noexcept
        : mesh(std::move(other.mesh)),
          shader(std::move(other.shader)),
          VAO(other.VAO),
          triangleEBO(other.triangleEBO),
          posVBO(other.posVBO),
          indicesCount(other.indicesCount)
    {
        other.VAO = 0;
        other.triangleEBO = 0;
        other.posVBO = 0;
        other.indicesCount = 0;
    }

    MeshRenderer& operator=(MeshRenderer&& other) noexcept
    {
        if (this != &other)
        {
            CleanupBuffers();

            mesh = std::move(other.mesh);
            shader = std::move(other.shader);
            VAO = other.VAO;
            triangleEBO = other.triangleEBO;
            posVBO = other.posVBO;
            indicesCount = other.indicesCount;

            other.VAO = 0;
            other.triangleEBO = 0;
            other.posVBO = 0;
            other.indicesCount = 0;
        }
        return *this;
    }

    ~MeshRenderer() { CleanupBuffers(); }

    void Draw() const
    {
        if (VAO == 0) return;

        glBindVertexArray(VAO);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(indicesCount),
            GL_UNSIGNED_INT,
            nullptr
        );

        glBindVertexArray(0);
    }

private:
    unsigned int VAO = 0;
    unsigned int triangleEBO = 0;

    unsigned int posVBO = 0;

    size_t indicesCount = 0;

    void InitializeBuffers()
    {
        if (!CheckMeshValidity()) return;

        indicesCount = mesh->triangles.size();

        glGenBuffers(1, &posVBO);
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(glm::vec3), mesh->vertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &triangleEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->triangles.size() * sizeof(unsigned int), mesh->triangles.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleEBO);
        glBindVertexArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void CleanupBuffers()
    {
        if (triangleEBO != 0)
        {
            glDeleteBuffers(1, &triangleEBO);
            triangleEBO = 0;
        }

        if (posVBO != 0)
        {
            glDeleteBuffers(1, &posVBO);
            posVBO = 0;
        }

        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
    }

    bool CheckMeshValidity() const
    {
        if (!mesh ||
            mesh->vertices.empty() ||
            mesh->triangles.empty())
        {
            return false;
        }

        return true;
    }
};
