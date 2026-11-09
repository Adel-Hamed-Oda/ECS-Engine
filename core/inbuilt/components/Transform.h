#pragma once

#include "core/private_imports.h"
#include "core/ecs/Component.h"

struct Transform {
public:
    Transform* parent = nullptr;
    std::vector<Transform*> children;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    Transform() 
        : parent(nullptr), position(0.0f), rotation(0.0f), scale(1.0f) {}

    Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
        : parent(nullptr), position(position), rotation(rotation), scale(scale) {}

    glm::vec3 Right() const { return glm::normalize(glm::vec3(GetRotationMatrix()[0])); }
    glm::vec3 Up() const { return glm::normalize(glm::vec3(GetRotationMatrix()[1])); }
        // because openGL says "fuck you" and uses -Z as forward
    glm::vec3 Forward() const { return glm::normalize(glm::vec3(GetRotationMatrix()[2]) * -1.0f); }

    glm::mat4 GetLocalRotationMatrix() const
    {
        glm::mat4 rotationMatrix(1.0f);

        rotationMatrix = glm::rotate(
            rotationMatrix,
            glm::radians(rotation.x),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );

        rotationMatrix = glm::rotate(
            rotationMatrix,
            glm::radians(rotation.y),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        
        rotationMatrix = glm::rotate(
            rotationMatrix,
            glm::radians(rotation.z),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        return rotationMatrix;
    }

    glm::mat4 GetRotationMatrix() const
    {
        glm::mat4 rotationMatrix = GetLocalRotationMatrix();

        if (parent) {
            rotationMatrix = parent->GetRotationMatrix() * rotationMatrix;
        }
        return rotationMatrix;
    }

    glm::mat4 GetLocalModelMatrix() const
    {
        glm::mat4 localModel(1.0f);

        localModel = glm::translate(localModel, position);
        localModel *= GetLocalRotationMatrix();
        localModel = glm::scale(localModel, scale);

        return localModel;
    }

    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 localModel = GetLocalModelMatrix();

        if (parent) {
            return parent->GetModelMatrix() * localModel;
        }
        return localModel;
    }

    glm::vec3 GetWorldPosition() const
    {
        if (parent) {
            return parent->GetWorldPosition() + position;
        }
        return position;
    }
    glm::vec3 GetWorldRotation() const
    {
        if (parent) {
            return parent->GetWorldRotation() + rotation;
        }
        return rotation;
    }
    glm::vec3 GetWorldScale() const
    {
        if (parent) {
            return parent->GetWorldScale() * scale;
        }
        return scale;
    }
};