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
#include "core/inbuilt/components/Animation.h"
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
        const glm::mat4& viewProj,
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

    void SendBonesToShader(const Animation* animation) const
    {
        if (!shader || !shader->IsValid()) return;

        if (!animation || !animation->HasValidBones())
        {
            shader->setBool("hasBones", false);
            return;
        }

        shader->setBool("hasBones", true);
        const auto& matrices = animation->finalBoneMatrices;
        const int boneCount = static_cast<int>(std::min(matrices.size(), static_cast<std::size_t>(Mesh::MAX_BONES)));
        for (int i = 0; i < boneCount; ++i)
        {
            shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", matrices[i]);
        }
    }

    bool SetupCoreUniforms(
        const Transform* transform,
        const Transform* cameraTransform,
        const glm::mat4& viewProj
    ) const {
		if (!transform || !shader || !shader->IsValid()) return false;

		if (!cameraTransform)
		{
			std::cerr << "ERROR::MATERIAL::NO_MAIN_CAMERA" << std::endl;
			return false;
		}

		shader->use();

		shader->setMat4("model", transform->GetModelMatrix());
		shader->setMat4("viewproj", viewProj);
		shader->setVec3("viewingPosition", cameraTransform->GetWorldPosition());

		return true;
	}

    bool SetupCoreUniforms(
        const Transform* transform, 
        const Transform* cameraTransform, 
        const Camera* camera, 
        const float aspectRatio
    ) const {
		if (!camera || !cameraTransform)
		{
			std::cerr << "ERROR::MATERIAL::NO_MAIN_CAMERA" << std::endl;
			return false;
		}

		return SetupCoreUniforms(transform, cameraTransform, camera->GetViewProjMatrix(cameraTransform, aspectRatio));
	}
};