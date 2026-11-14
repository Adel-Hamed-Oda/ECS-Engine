#pragma once

#include "core/private_imports.h"
#include "core/rendering/shader/Shader.h"
#include "core/inbuilt/components/Transform.h"
#include "core/inbuilt/components/Camera.h"
#include "Material.h"

class ScreenspaceMaterial : public Material {
public:
    explicit ScreenspaceMaterial(std::shared_ptr<Shader> shader) : Material(shader) {}
    ~ScreenspaceMaterial() override = default;

    virtual void ApplyMaterial() override = 0;

    bool SetupScreenSpaceUniforms(const Transform* transform) const
	{
		if (!transform || !shader || !shader->IsValid()) return false;

		const glm::mat4 orthoProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, -10.0f, 10.0f);

		shader->use();

		shader->setMat4("model", transform->GetModelMatrix());
		shader->setMat4("view", glm::mat4(1.0f));
		shader->setMat4("projection", orthoProjection);

		return true;
	}
};