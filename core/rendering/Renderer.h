#pragma once

#include <glad/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <string>
#include <algorithm>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/scenes/Scene.h"
#include "core/inbuilt/components/LightSource.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Transform2D.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/MeshRenderer.h"
#include "core/inbuilt/components/Animation.h"
#include "core/inbuilt/components/TextRenderer.h"
#include "core/inbuilt/components/ImageRenderer.h"
#include "core/rendering/material/ImageMaterial.h"
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

        Registry& registry = scene->GetRegistry();

        const Camera& cam = registry.HasComponent<Camera>(cameraEntity)
            ? registry.GetComponent<Camera>(cameraEntity)
            : Camera();
        const Transform& camTransform = registry.HasComponent<Transform>(cameraEntity)
            ? registry.GetComponent<Transform>(cameraEntity)
            : Transform();
        const glm::mat4 viewProj = cam.GetViewProjMatrix(&camTransform, aspectRatio);

        registry.ForEach<Transform, LightSource>([&](Entity e, Transform& transform, LightSource& lightSource) {
            Light* light = new Light();
            light->lightSource = &lightSource;
            light->transform = &transform;
            lights.push_back(light);
        });

        // Normal rendering
        registry.ForEach<Transform, MeshRenderer>([&](Entity e, Transform& transform, MeshRenderer& meshRenderer) {
            if (registry.HasComponent<Animation>(e)) return;
            meshRenderer.Draw(&transform, &camTransform, viewProj, lights);
        });
        // Animated rendering
        registry.ForEach<Transform, MeshRenderer, Animation>([&](Entity e, Transform& transform, MeshRenderer& meshRenderer, Animation& animation) {
            meshRenderer.Draw(&transform, &camTransform, viewProj, lights, &animation);
        });

        // default orthographic projection for 2D rendering (cause why not)
        const glm::mat4 orthoProjection = glm::ortho( 0.0f, static_cast<float>(viewportWidth), 0.0f, static_cast<float>(viewportHeight) );

        // render screen-space images
        Enable2DState();
        registry.ForEach<Transform2D, ImageRenderer>([&](Entity e, Transform2D& transform2D, ImageRenderer& imageRenderer) {
            imageRenderer.Draw(&transform2D, orthoProjection);
        });

        // render screen-space text
        Enable2DState();
        registry.ForEach<Transform2D, TextRenderer>([&](Entity e, Transform2D& transform2D, TextRenderer& textRenderer) {
            textRenderer.Draw(&transform2D, orthoProjection);
        });

        Disable2DState();

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

    inline static bool screenspace2DStateSet = false;
    inline static GLboolean depthEnabled = GL_FALSE;
    inline static GLboolean depthMask = GL_TRUE;

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

    static void Enable2DState() {
        if (!screenspace2DStateSet)
        {
            depthEnabled = glIsEnabled(GL_DEPTH_TEST);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            screenspace2DStateSet = true;
        }
    }

    static void Disable2DState() {
        if (screenspace2DStateSet)
        {
            glDepthMask(depthMask);
            if (depthEnabled) glEnable(GL_DEPTH_TEST);
            else glDisable(GL_DEPTH_TEST);
        }
    }
};
