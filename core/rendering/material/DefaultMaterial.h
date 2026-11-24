#pragma once

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

#include "WorldspaceMaterial.h"
#include "core/rendering/texture/Texture.h"
#include "core/asset_management/FileReader.h"
#include "core/FallBacks.h"

class DefaultMaterial : public WorldspaceMaterial {
public:
	Texture* texture = nullptr;
	glm::vec4 baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 ambient = glm::vec3(0.1f);
	float specularStrength = 0.5f;
	float shininess = 32.0f;

	explicit DefaultMaterial(std::shared_ptr<Shader> shader = GetDefaultShader()) 
		: WorldspaceMaterial(std::move(shader)) {}
	~DefaultMaterial() override = default;

	using WorldspaceMaterial::ApplyMaterial;

	void ApplyMaterial(
		const Transform* transform,
		const Transform* cameraTransform,
		const glm::mat4& viewProj,
		const std::vector<Light*>& lights
	) override
	{
		if (!SetupCoreUniforms(transform, cameraTransform, viewProj)) return;

		SendLightsToShader(lights);

		const bool hasTexture = texture && texture->IsValid();
		if (hasTexture)
		{
			texture->Bind(0);
		}
		else
		{
			Texture::Unbind(0);
		}

		shader->setBool("material.hasTexture", hasTexture);
		shader->setInt("material.textureSampler", 0);
		
		shader->setVec4("material.baseColor", baseColor);
		shader->setVec3("material.ambient", ambient);
		shader->setFloat("material.specularStrength", specularStrength);
		shader->setFloat("material.shininess", shininess);
	}

	static std::shared_ptr<Shader> GetDefaultShader()
	{
		if (defaultShader && defaultShader->IsValid())
		{
			return defaultShader;
		}

		std::string vertCode = FileReader::Read("assets/Default.vert");
		std::string fragCode = FileReader::Read("assets/Default.frag");

		if (!vertCode.empty() && !fragCode.empty())
		{
			defaultShader = std::make_shared<Shader>(vertCode, fragCode);
		}

		return defaultShader;
	}

private:
	inline static std::shared_ptr<Shader> defaultShader = nullptr;
};