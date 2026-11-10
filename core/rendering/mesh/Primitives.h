#pragma once

#include "core/private_imports.h"
#include "Mesh.h"

class Primitives {
    static std::unique_ptr<Mesh> CreateCube(
        glm::vec3 dimensions = glm::vec3(1.0f),
        glm::vec4 color = glm::vec4(1.0f)
    )
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

        mesh->vertices = {
            // Front (+Z)
            glm::vec3(-0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f),

            // Back (-Z)
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),

            // Right (+X)
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),

            // Left (-X)
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(-0.5f, -0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f,  0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),

            // Top (+Y)
            glm::vec3(-0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f,  0.5f),
            glm::vec3(0.5f,  0.5f, -0.5f),
            glm::vec3(-0.5f,  0.5f, -0.5f),

            // Bottom (-Y)
            glm::vec3(-0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f, -0.5f),
            glm::vec3(0.5f, -0.5f,  0.5f),
            glm::vec3(-0.5f, -0.5f,  0.5f)
        };
        
        for (auto& vertex : mesh->vertices) {
            vertex *= dimensions;
        }

        mesh->triangles = {
            // Front
            0,  1,  2,
            2,  3,  0,

            // Back
            4,  5,  6,
            6,  7,  4,

            // Right
            8,  9, 10,
            10, 11,  8,

            // Left
            12, 13, 14,
            14, 15, 12,

            // Top
            16, 17, 18,
            18, 19, 16,

            // Bottom
            20, 21, 22,
            22, 23, 20
        };

        mesh->colors = {
            color, color, color, color,
            color, color, color, color,
            color, color, color, color,
            color, color, color, color,
            color, color, color, color,
            color, color, color, color
        };

        mesh->normals = {
            // Front (+Z)
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f),

            // Back (-Z)
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),

            // Right (+X)
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),

            // Left (-X)
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 0.0f),

            // Top (+Y)
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),

            // Bottom (-Y)
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)
        };

        mesh->uvs = {
            // Front
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            // Back
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            // Right
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            // Left
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            // Top
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),

            // Bottom
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f)
        };

        return std::move(mesh);
    }

    static std::unique_ptr<Mesh> CreateSphere(
        float radius = 0.5f,
        unsigned int latitudeBands = 20,
        unsigned int longitudeBands = 20,
        glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    )
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

        for (unsigned int lat = 0; lat <= latitudeBands; ++lat)
        {
            float theta = lat * glm::pi<float>() / latitudeBands;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            for (unsigned int lon = 0; lon <= longitudeBands; ++lon)
            {
                float phi = lon * 2.0f * glm::pi<float>() / longitudeBands;
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                // Outward-facing unit normal (same convention as the cube)
                glm::vec3 normal(
                    cosPhi * sinTheta,
                    cosTheta,
                    sinPhi * sinTheta
                );

                glm::vec2 uv(
                    (float)lon / (float)longitudeBands,
                    1.0f - (float)lat / (float)latitudeBands // v increases upward, like the cube
                );

                mesh->vertices.push_back(normal * radius);
                mesh->normals.push_back(normal);
                mesh->uvs.push_back(uv);
                mesh->colors.push_back(color);
            }
        }

        for (unsigned int lat = 0; lat < latitudeBands; ++lat)
        {
            for (unsigned int lon = 0; lon < longitudeBands; ++lon)
            {
                unsigned int first = (lat * (longitudeBands + 1)) + lon;
                unsigned int second = first + longitudeBands + 1;

                // First triangle of quad
                mesh->triangles.push_back(first);
                mesh->triangles.push_back(first + 1);
                mesh->triangles.push_back(second);

                // Second triangle of quad
                mesh->triangles.push_back(second);
                mesh->triangles.push_back(first + 1);
                mesh->triangles.push_back(second + 1);
            }
        }

        return std::move(mesh);
    }

    static std::unique_ptr<Mesh> CreateQuad(
        glm::vec2 dimensions = glm::vec2(1.0f),
        glm::vec4 color = glm::vec4(1.0f)
    )
    {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

        mesh->vertices = {
            // Bottom-left
            glm::vec3(-dimensions.x * 0.5f, -dimensions.y * 0.5f, 0.0f),

            // Bottom-right
            glm::vec3( dimensions.x * 0.5f, -dimensions.y * 0.5f, 0.0f),

            // Top-right
            glm::vec3( dimensions.x * 0.5f,  dimensions.y * 0.5f, 0.0f),

            // Top-left
            glm::vec3(-dimensions.x * 0.5f,  dimensions.y * 0.5f, 0.0f)
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

        return std::move(mesh);
    }
};