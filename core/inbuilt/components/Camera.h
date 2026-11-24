#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

    glm::mat4 GetProjectionMatrix(float aspectRatio) const
    {
        return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }

    glm::mat4 GetViewProjMatrix(const Transform* transform, float aspectRatio) const
    {
        if (!transform) return glm::mat4(1.0f);
        glm::mat4 view = GetViewMatrix(transform);
        glm::mat4 projection = GetProjectionMatrix(aspectRatio);

        return projection * view;
    }
};