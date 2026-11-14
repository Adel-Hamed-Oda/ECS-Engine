#pragma once

#include "core/private_imports.h"
#include "WorldspaceMaterial.h"

class DefaultMaterial : public WorldspaceMaterial {
public:
	//Texture* texture = nullptr;
	glm::vec4 baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	float specularStrength = 0.5f;
	float shininess = 32.0f;

	explicit DefaultMaterial(std::shared_ptr<Shader> shader) : WorldspaceMaterial(shader) {}
	~DefaultMaterial() override = default;

	void ApplyMaterial() override
	{
		if (!shader || !shader->IsValid()) return;

		//const bool hasTexture = texture && texture->IsValid();
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, hasTexture ? texture->ID : 0);

		//shader->setBool("material.hasTexture", hasTexture);
		//shader->setInt("material.textureSampler", 0);
		shader->setVec4("material.baseColor", baseColor);
		shader->setVec3("material.ambient", ambient);
		shader->setFloat("material.specularStrength", specularStrength);
		shader->setFloat("material.shininess", shininess);
	}
};