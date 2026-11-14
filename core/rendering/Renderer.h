#pragma once

#include "core/private_imports.h"
#include "core/scenes/Scene.h"
#include "core/inbuilt/components/LightSource.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Transform2D.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/MeshRenderer.h"
#include "core/inbuilt/components/TextRenderer.h"
#include "core/rendering/font/Font.h"
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

        if (!ft && FT_Init_FreeType(&ft) == 0) // Init FreeType library
        {
            Font::ft = ft;
        }
    }

    static void RenderScene(
        Scene* scene,
        int viewportWidth = 1920,
        int viewportHeight = 1080,
        Entity cameraEntity = NULL_ENTITY
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

        const Camera& cam = registry.HasComponent<Camera>(cameraEntity)
            ? registry.GetComponent<Camera>(cameraEntity)
            : Camera();
        const Transform& camTransform = registry.HasComponent<Transform>(cameraEntity)
            ? registry.GetComponent<Transform>(cameraEntity)
            : Transform();
        view = cam.GetViewMatrix(camTransform);
        projection = cam.GetProjectionMatrix(aspectRatio);

        registry.ForEach<Transform, LightSource>([&](Entity e, Transform& transform, LightSource& lightSource) {
            if (e == NULL_ENTITY) return;

            Light* light = new Light();
            light->lightSource = &lightSource;
            light->transform = &transform;
            lights.push_back(light);
        });

        registry.ForEach<Transform, MeshRenderer>([&](Entity e, Transform& transform, MeshRenderer& meshRenderer) {
            if (e == NULL_ENTITY) return;

            meshRenderer.Draw(&transform, &camTransform, &cam, aspectRatio, lights);
        });

        const glm::mat4 orthoProjection = glm::ortho( 0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight) );

        bool textStateSet = false;
        GLboolean depthEnabled = GL_FALSE;
        GLboolean depthMask = GL_TRUE;

        registry.ForEach<Transform2D, TextRenderer>([&](Entity e, Transform2D& transform2D, TextRenderer& textRenderer) {
            if (e == NULL_ENTITY) return;

            if (!textStateSet)
            {
                depthEnabled = glIsEnabled(GL_DEPTH_TEST);
                glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                textStateSet = true;
            }
            ScreenspaceMaterial* activeMaterial = textRenderer.material.get();

            activeMaterial->shader->use();
            activeMaterial->shader->setMat4("model", transform2D.GetModelMatrix4());
            activeMaterial->shader->setMat4("view", glm::mat4(1.0f));
            activeMaterial->shader->setMat4("projection", orthoProjection);

            textRenderer.Draw(transform2D.position.x, transform2D.position.y, transform2D.scale);
        });

        if (textStateSet)
        {
            glDepthMask(depthMask);
            if (depthEnabled) glEnable(GL_DEPTH_TEST);
            else glDisable(GL_DEPTH_TEST);
        }

        // Clean up lights
        for (Light* light : lights)
        {
            delete light;
        }
        lights.clear();
    }

private:
    inline static FT_Library ft = nullptr;
    inline static std::vector<Light*> lights;

    static void SendLights(const Shader* shader)
    {
        if (!shader || !shader->IsValid()) return;

        const int maxLights = 256; // Maximum number of lights MUST BE A CONSTANT
        const int lightCount = static_cast<int>( std::min(lights.size(), static_cast<std::size_t>(maxLights)) );

        shader->setInt("numLights", lightCount);

        for (int i = 0; i < lightCount; ++i)
        {
            Light* light = lights[i];

            const std::string prefix = "lights[" + std::to_string(i) + "].";

            shader->setVec3( prefix + "position", light->transform->position );
            shader->setVec3( prefix + "color", light->lightSource->color );
        }
    }
};
