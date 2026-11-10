#pragma once

#include "core/private_imports.h"
#include "core/rendering/mesh/Mesh.h"

struct MeshRenderer
{
public:
    std::unique_ptr<Mesh> mesh;

    MeshRenderer(std::unique_ptr<Mesh> mesh) : mesh(std::move(mesh)) { InitializeBuffers(); }
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
