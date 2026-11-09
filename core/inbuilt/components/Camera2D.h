#pragma once

#include "core/private_imports.h"
#include "Transform2D.h"

struct Camera2D {
public:
    inline static Camera2D* mainCamera = nullptr;

    float orthographicSize = 5.0f;
    float nearPlane = -100.0f;
    float farPlane = 100.0f;

    Camera2D(float orthographicSize = 5.0f, float nearPlane = -100.0f, float farPlane = 100.0f)
        : orthographicSize(orthographicSize), nearPlane(nearPlane), farPlane(farPlane)
    {
        Register();
    }

    ~Camera2D()
    {
        Unregister();
    }

    glm::mat4 GetViewMatrix(const Transform2D* transform) const
    {
        if (!transform) return glm::mat4(1.0f);

        glm::vec2 pos = transform->GetWorldPosition();
        float rot = transform->GetWorldRotation();

        glm::mat4 view = glm::mat4(1.0f);

        view = glm::rotate(view, glm::radians(-rot), glm::vec3(0.0f, 0.0f, 1.0f));
        view = glm::translate(view, glm::vec3(-pos.x, -pos.y, 0.0f));

        return view;
    }

    glm::mat4 GetViewMatrix(const Transform2D& transform) const
    {
        return GetViewMatrix(&transform);
    }

    glm::mat4 GetProjectionMatrix(float aspectRatio) const
    {
        float top = orthographicSize;
        float bottom = -orthographicSize;
        float right = orthographicSize * aspectRatio;
        float left = -orthographicSize * aspectRatio;

        return glm::ortho(left, right, bottom, top, nearPlane, farPlane);
    }

private:
    inline static std::vector<Camera2D*> cameras;

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
