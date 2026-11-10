#pragma once

#include "core/private_imports.h"

struct Mesh
{
public:
    Mesh() = default;

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> triangles;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec2> uvs;

    static Mesh* CreateQuad(
        glm::vec4 color = glm::vec4(1.0f)
    )
    {
        Mesh* mesh = new Mesh();

        mesh->vertices = {
            // Bottom-left
            glm::vec3(-0.5f, -0.5f, 0.0f),

            // Bottom-right
            glm::vec3( 0.5f, -0.5f, 0.0f),

            // Top-right
            glm::vec3( 0.5f,  0.5f, 0.0f),

            // Top-left
            glm::vec3(-0.5f,  0.5f, 0.0f)
        };

        mesh->triangles = {
            0, 1, 2,
            2, 3, 0
        };

        mesh->normals = {
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        };

        mesh->colors = {
            color,
            color,
            color,
            color
        };

        mesh->uvs = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
        };

        return mesh;
    }
};
