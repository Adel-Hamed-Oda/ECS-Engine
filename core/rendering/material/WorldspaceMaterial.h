#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstddef>

#include "core/rendering/shader/Shader.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Camera.h"
#include "core/inbuilt/components/LightSource.h"
#include "Material.h"

struct Light {
    Transform* transform = nullptr;
    LightSource* lightSource = nullptr;
};

class WorldspaceMaterial : public Material {
public:
    explicit WorldspaceMaterial(std::shared_ptr<Shader> shader) : Material(shader) {}
    ~WorldspaceMaterial() override = default;

    virtual void ApplyMaterial(
        const Transform* transform,
        const Transform* cameraTransform,
        const Camera* camera,
        float aspectRatio,
        const std::vector<Light*>& lights
    ) = 0;

    void SendLightsToShader(const std::vector<Light*>& lights) const
    {
        if (!shader || !shader->IsValid()) return;

        const int maxLights = 256; // Maximum number of lights MUST BE A CONSTANT
        const int lightCount = static_cast<int>( std::min(lights.size(), static_cast<std::size_t>(maxLights)) );

        shader->setInt("numLights", lightCount);

        for (int i = 0; i < lightCount; ++i)
        {
            Light* light = lights[i];
            if (!light || !light->transform || !light->lightSource) continue;

            const std::string prefix = "lights[" + std::to_string(i) + "].";

            shader->setVec3( prefix + "position", light->transform->position );
            shader->setVec3( prefix + "color", light->lightSource->color );
        }
    }

    bool SetupCoreUniforms(
        const Transform* transform, 
        const Transform* cameraTransform, 
        const Camera* camera, 
        const float aspectRatio
    ) const {
		if (!transform || !shader || !shader->IsValid()) return false;

		if (!camera || !cameraTransform)
		{
			std::cerr << "ERROR::MATERIAL::NO_MAIN_CAMERA" << std::endl;
			return false;
		}

		shader->use();

		shader->setMat4("model", transform->GetModelMatrix());
		shader->setMat4("view", camera->GetViewMatrix(cameraTransform));
		shader->setMat4("projection", camera->GetProjectionMatrix(aspectRatio));
		shader->setVec3("viewingPosition", cameraTransform->GetWorldPosition());

		return true;
	}
};