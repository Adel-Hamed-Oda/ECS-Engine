#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/ecs/Component.h"

struct Transform {
public:
    Transform* parent = nullptr;
    std::vector<Transform*> children;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    mutable bool dirty = true;

    Transform() 
        : parent(nullptr), position(0.0f), rotation(0.0f), scale(1.0f), dirty(true) {}

    Transform(glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f))
        : parent(nullptr), position(position), rotation(rotation), scale(scale), dirty(true) {}

    void MarkDirty() const
    {
        dirty = true;
        for (Transform* child : children) {
            if (child) {
                child->MarkDirty();
            }
        }
    }

    bool IsDirty() const
    {
        return dirty || position != lastPosition || rotation != lastRotation || scale != lastScale;
    }

    glm::vec3 Right() const { return glm::normalize(glm::vec3(GetRotationMatrix()[0])); }
    glm::vec3 Up() const { return glm::normalize(glm::vec3(GetRotationMatrix()[1])); }
        // because openGL says "fuck you" and uses -Z as forward
    glm::vec3 Forward() const { return glm::normalize(glm::vec3(GetRotationMatrix()[2]) * -1.0f); }

    const glm::mat4& GetLocalRotationMatrix() const
    {
        UpdateMatrices();
        return localRotationMatrix;
    }

    glm::mat4 GetRotationMatrix() const
    {
        const glm::mat4& rotationMatrix = GetLocalRotationMatrix();

        if (parent) {
            return parent->GetRotationMatrix() * rotationMatrix;
        }
        return rotationMatrix;
    }

    const glm::mat4& GetLocalModelMatrix() const
    {
        UpdateMatrices();
        return localModelMatrix;
    }

    glm::mat4 GetModelMatrix() const
    {
        const glm::mat4& localModel = GetLocalModelMatrix();

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

private:
    mutable glm::mat4 localRotationMatrix = glm::mat4(1.0f);
    mutable glm::mat4 localModelMatrix = glm::mat4(1.0f);

    mutable glm::vec3 lastPosition = glm::vec3(0.0f);
    mutable glm::vec3 lastRotation = glm::vec3(0.0f);
    mutable glm::vec3 lastScale = glm::vec3(1.0f);

    void UpdateMatrices() const
    {
        if (!IsDirty()) {
            return;
        }

        if (rotation != lastRotation) {
            glm::mat4 rot(1.0f);

            // Apply Yaw (Y) -> Pitch (X) -> Roll (Z) so local X pitch stays level relative to the horizon
            // otherwise there are issues that will happen with the camera (the issue is called "gimbal locks"
            // and the only solution is using Quaternions, which I am NOT that much of a masochist to do)
            rot = glm::rotate(
                rot,
                glm::radians(rotation.y),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            rot = glm::rotate(
                rot,
                glm::radians(rotation.x),
                glm::vec3(1.0f, 0.0f, 0.0f)
            );

            rot = glm::rotate(
                rot,
                glm::radians(rotation.z),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );

            localRotationMatrix = rot;
            lastRotation = rotation;
        }

        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model *= localRotationMatrix;
        model = glm::scale(model, scale);

        localModelMatrix = model;
        lastPosition = position;
        lastScale = scale;
        dirty = false;
    }
};