#pragma once

#include "core/private_imports.h"
#include "core/rendering/shader/Shader.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Camera.h"
#include "Material.h"

class WorldspaceMaterial : public Material {
public:
    explicit WorldspaceMaterial(std::shared_ptr<Shader> shader) : Material(shader) {}
    ~WorldspaceMaterial() override = default;

    virtual void ApplyMaterial() override = 0;

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
		shader->setVec3("viewingPosition", cameraTransform->position);

		return true;
	}
};