#pragma once

#include "core/private_imports.h"
#include "core/ecs/Component.h"

struct Transform2D {
public:
    Transform2D* parent = nullptr;
    std::vector<Transform2D*> children;

    glm::vec2 position = glm::vec2(0.0f);
    float rotation = 0.0f; // Rotation angle in degrees
    glm::vec2 scale = glm::vec2(1.0f);

    Transform2D()
        : parent(nullptr), position(0.0f), rotation(0.0f), scale(1.0f) {}

    Transform2D(glm::vec2 position, float rotation = 0.0f, glm::vec2 scale = glm::vec2(1.0f))
        : parent(nullptr), position(position), rotation(rotation), scale(scale) {}

    glm::vec2 Right() const { return glm::normalize(glm::vec2(GetRotationMatrix()[0])); }
    glm::vec2 Up() const { return glm::normalize(glm::vec2(GetRotationMatrix()[1])); }

    glm::mat3 GetLocalRotationMatrix() const
    {
        float rad = glm::radians(rotation);
        float c = std::cos(rad);
        float s = std::sin(rad);

        return glm::mat3(
            c,    s, 0.0f,
           -s,    c, 0.0f,
         0.0f, 0.0f, 1.0f
        );
    }

    glm::mat3 GetRotationMatrix() const
    {
        glm::mat3 rotationMatrix = GetLocalRotationMatrix();

        if (parent) {
            rotationMatrix = parent->GetRotationMatrix() * rotationMatrix;
        }
        return rotationMatrix;
    }

    glm::mat3 GetLocalModelMatrix() const
    {
        float rad = glm::radians(rotation);
        float c = std::cos(rad);
        float s = std::sin(rad);

        // WHO TF THOUGHT THEY SHOULD REPLACE ROWS AND COLUMNS OMG
        return glm::mat3(
            c * scale.x,  s * scale.x, 0.0f,
           -s * scale.y,  c * scale.y, 0.0f,
             position.x,   position.y, 1.0f
        );
    }

    glm::mat3 GetModelMatrix() const
    {
        glm::mat3 localModel = GetLocalModelMatrix();

        if (parent)
        {
            return parent->GetModelMatrix() * localModel;
        }
        return localModel;
    }

    // Converts the 2D 3x3 affine transformation matrix into a 4x4 matrix suitable for standard OpenGL shaders
    glm::mat4 GetModelMatrix4() const
    {
        glm::mat3 m = GetModelMatrix();
        return glm::mat4(
            glm::vec4(m[0].x, m[0].y, 0.0f, 0.0f),
            glm::vec4(m[1].x, m[1].y, 0.0f, 0.0f),
            glm::vec4(0.0f,   0.0f,   1.0f, 0.0f),
            glm::vec4(m[2].x, m[2].y, 0.0f, 1.0f)
        );
    }

    glm::vec2 GetWorldPosition() const
    {
        if (parent)
        {
            return glm::vec2(parent->GetModelMatrix() * glm::vec3(position, 1.0f));
        }
        return position;
    }
    float GetWorldRotation() const
    {
        if (parent)
        {
            return parent->GetWorldRotation() + rotation;
        }
        return rotation;
    }
    glm::vec2 GetWorldScale() const
    {
        if (parent)
        {
            return parent->GetWorldScale() * scale;
        }
        return scale;
    }
};
