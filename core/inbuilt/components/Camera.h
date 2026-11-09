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
        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(view, glm::radians(-transform->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(-transform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(-transform->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        view = glm::translate(view, -pos);

        return view;
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