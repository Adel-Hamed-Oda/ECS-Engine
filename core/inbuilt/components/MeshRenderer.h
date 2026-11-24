#pragma once

#include <glad/gl.h>
#include <memory>
#include <vector>
#include <utility>
#include <cstddef>

#include "core/rendering/buffers/VAO.h"
#include "core/rendering/buffers/VBO.h"
#include "core/rendering/buffers/EBO.h"
#include "core/rendering/mesh/Mesh.h"
#include "core/rendering/shader/Shader.h"
#include "core/rendering/material/WorldspaceMaterial.h"

struct MeshRenderer
{
public:
    std::unique_ptr<Mesh> mesh;
    std::shared_ptr<WorldspaceMaterial> material;

    MeshRenderer(std::unique_ptr<Mesh> mesh, std::shared_ptr<WorldspaceMaterial> material = nullptr)
        : mesh(std::move(mesh)), material(std::move(material))
    {
        InitializeBuffers();
    }

    MeshRenderer(const MeshRenderer&) = delete;
    MeshRenderer& operator=(const MeshRenderer&) = delete;

    MeshRenderer(MeshRenderer&& other) noexcept
        : mesh(std::move(other.mesh)),
          material(std::move(other.material)),
          vao(std::move(other.vao)),
          posVBO(std::move(other.posVBO)),
          triangleEBO(std::move(other.triangleEBO)),
          normalVBO(std::move(other.normalVBO)),
          colorVBO(std::move(other.colorVBO)),
          uvVBO(std::move(other.uvVBO)),
          indicesCount(other.indicesCount)
    {
        other.indicesCount = 0;
    }

    MeshRenderer& operator=(MeshRenderer&& other) noexcept
    {
        if (this != &other)
        {
            CleanupBuffers();

            mesh = std::move(other.mesh);
            material = std::move(other.material);
            vao = std::move(other.vao);
            posVBO = std::move(other.posVBO);
            triangleEBO = std::move(other.triangleEBO);
            normalVBO = std::move(other.normalVBO);
            colorVBO = std::move(other.colorVBO);
            uvVBO = std::move(other.uvVBO);
            indicesCount = other.indicesCount;

            other.indicesCount = 0;
        }
        return *this;
    }

    ~MeshRenderer() { CleanupBuffers(); }

    void Draw(
        const Transform* transform,
        const Transform* cameraTransform,
        const glm::mat4& viewProj,
        const std::vector<Light*>& lights
    ) const
    {
        if (!vao.IsValid()) return;

        if (material)
        {
            material->ApplyMaterial(transform, cameraTransform, viewProj, lights);
        }

        vao.Bind();

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(indicesCount),
            GL_UNSIGNED_INT,
            nullptr
        );

        VAO::Unbind();
    }

private:
    VAO vao;
    VBO posVBO;
    EBO triangleEBO;
    VBO normalVBO;
    VBO colorVBO;
    VBO uvVBO;

    size_t indicesCount = 0;

    void InitializeBuffers()
    {
        if (!CheckMeshValidity())
        {
            CleanupBuffers();
            return;
        }

        indicesCount = mesh->triangles.size();

        if (!vao.IsValid()) vao.Create();
        vao.Bind();

        posVBO = VBO(mesh->vertices);
        vao.LinkAttrib(posVBO, 0, 3, GL_FLOAT, sizeof(glm::vec3), nullptr); 

        triangleEBO = EBO(mesh->triangles);
        vao.LinkEBO(triangleEBO);

        normalVBO = VBO(mesh->normals);
        vao.LinkAttrib(normalVBO, 1, 3, GL_FLOAT, sizeof(glm::vec3), nullptr);

        colorVBO = VBO(mesh->colors);
        vao.LinkAttrib(colorVBO, 2, 4, GL_FLOAT, sizeof(glm::vec4), nullptr);

        uvVBO = VBO(mesh->uvs);
        vao.LinkAttrib(uvVBO, 3, 2, GL_FLOAT, sizeof(glm::vec2), nullptr);

        // Unbind VAO before unbinding EBO so the VAO preserves its EBO binding
        VAO::Unbind();
        VBO::Unbind();
        EBO::Unbind();
    }

    void CleanupBuffers()
    {
        triangleEBO.Delete();
        normalVBO.Delete();
        colorVBO.Delete();
        uvVBO.Delete();
        posVBO.Delete();
        vao.Delete();
        indicesCount = 0;
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
