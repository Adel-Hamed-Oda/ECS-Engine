#pragma once

#include "core/private_imports.h"
#include "core/scenes/Scene.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/MeshRenderer.h"
#include "core/rendering/shader/Shader.h"

class Renderer
{
public:
    static void SetupRendering()
    {
        if (!glad_glEnable) return;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
                    
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // Configure alpha blending (something to do with transparency)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    static void RenderScene(
        Scene* scene,
        int viewportWidth = 1920,
        int viewportHeight = 1080,
        Entity cameraEntity = NULL_ENTITY,
        const Shader* overrideShader = nullptr
    )
    {
        if (!scene)
            return;

        float aspectRatio = (viewportHeight > 0)
            ? static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight)
            : 1.0f;

        glm::mat4 view(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);

        Registry& registry = scene->GetRegistry();
        bool cameraFound = false;

        if (cameraEntity != NULL_ENTITY && registry.IsAlive(cameraEntity) && registry.HasComponent<Camera>(cameraEntity))
        {
            const Camera& cam = registry.GetComponent<Camera>(cameraEntity);
            const Transform* camTransform = registry.HasComponent<Transform>(cameraEntity)
                ? &registry.GetComponent<Transform>(cameraEntity)
                : nullptr;
            view = cam.GetViewMatrix(camTransform);
            projection = cam.GetProjectionMatrix(aspectRatio);
            cameraFound = true;
        }
        else
        {
            registry.ForEach<Camera>([&](Entity e, Camera& cam) {
                if (cameraFound) return;
                const Transform* camTransform = registry.HasComponent<Transform>(e)
                    ? &registry.GetComponent<Transform>(e)
                    : nullptr;
                view = cam.GetViewMatrix(camTransform);
                projection = cam.GetProjectionMatrix(aspectRatio);
                cameraFound = true;
            });
        }

        registry.ForEach<Transform, MeshRenderer>([&](Entity e, Transform& transform, MeshRenderer& meshRenderer) {
            if (e == NULL_ENTITY) return;

            const Shader* activeShader = meshRenderer.shader ? meshRenderer.shader.get() : overrideShader;
            if (activeShader && activeShader->IsValid())
            {
                activeShader->use();
                activeShader->setMat4("model", transform.GetModelMatrix());
                activeShader->setMat4("view", view);
                activeShader->setMat4("projection", projection);
            }

            meshRenderer.Draw();
        });
    }
};
