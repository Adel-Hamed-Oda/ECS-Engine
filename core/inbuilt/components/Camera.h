#pragma once

#include "core/private_imports.h"
#include "Transform.h"

struct Camera {
public:
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    Camera(float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 1000.0f)
        : fov(fov), nearPlane(nearPlane), farPlane(farPlane) {}

    ~Camera() = default;

    glm::mat4 GetViewMatrix(const Transform* transform) const
    {
        if (!transform) return glm::mat4(1.0f);

        glm::vec3 pos = transform->GetWorldPosition();
        glm::mat4 invRotation = glm::transpose(transform->GetRotationMatrix());

        return glm::translate(invRotation, -pos);
    }

    glm::mat4 GetViewMatrix(const Transform& transform) const
    {
        return GetViewMatrix(&transform);
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio) const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
};