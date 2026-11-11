#pragma once

#include "core/private_imports.h"
#include "Transform.h"

struct Camera {
public:
    inline static Camera* mainCamera = nullptr;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;

    Camera(float fov = 45.0f, float nearPlane = 0.1f, float farPlane = 1000.0f)
        : fov(fov), nearPlane(nearPlane), farPlane(farPlane)
    {
        Register();
    }

    ~Camera()
    {
        Unregister();
    }

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

private:
    inline static std::vector<Camera*> cameras;

    void Register()
    {
        if (!mainCamera)
            mainCamera = this;
        cameras.push_back(this);
    }

    void Unregister()
    {
        auto it = std::find(cameras.begin(), cameras.end(), this);
        if (it != cameras.end()) {
            cameras.erase(it);
        }

        if (mainCamera == this) {
            mainCamera = cameras.empty() ? nullptr : cameras.front();
        }
    }
};