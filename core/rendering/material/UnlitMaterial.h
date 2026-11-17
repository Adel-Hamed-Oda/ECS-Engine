#pragma once

#include "core/private_imports.h"
#include "WorldspaceMaterial.h"
#include "core/rendering/texture/Texture.h"
#include "core/asset_management/FileReader.h"
#include "core/rendering/shader/UnlitFallBack.h"

class UnlitMaterial : public WorldspaceMaterial {
public:
	Texture* texture = nullptr;
	glm::vec4 baseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	explicit UnlitMaterial(std::shared_ptr<Shader> shader = nullptr)
		: WorldspaceMaterial(std::move(shader))
	{
		if (!this->shader)
		{
			this->shader = GetDefaultShader();
		}
	}

	~UnlitMaterial() override = default;

	void ApplyMaterial(
		const Transform* transform,
		const Transform* cameraTransform,
		const Camera* camera,
		float aspectRatio,
		const std::vector<Light*>& lights
	) override
	{
		if (!SetupCoreUniforms(transform, cameraTransform, camera, aspectRatio)) return;

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

		// Also set top-level uniform names
		shader->setBool("hasTexture", hasTexture);
		shader->setInt("textureSampler", 0);
		shader->setVec4("color", baseColor);
	}

	static std::shared_ptr<Shader> GetDefaultShader()
	{
		if (defaultShader && defaultShader->IsValid())
		{
			return defaultShader;
		}

		std::string vertCode = FileReader::ReadFirstExisting({
			"assets/Unlit.vert",
			"../assets/Unlit.vert",
			"../../assets/Unlit.vert"
		});

		std::string fragCode = FileReader::ReadFirstExisting({
			"assets/Unlit.frag",
			"../assets/Unlit.frag",
			"../../assets/Unlit.frag"
		});

		if (!vertCode.empty() && !fragCode.empty())
		{
			defaultShader = Shader::CreateFromSource(vertCode, fragCode);
			if (defaultShader && defaultShader->IsValid())
			{
				return defaultShader;
			}
		}

		defaultShader = Shader::CreateFromSource(
			UnlitFallBack::VertexShaderCode,
			UnlitFallBack::FragmentShaderCode
		);
		return defaultShader;
	}

private:
	inline static std::shared_ptr<Shader> defaultShader = nullptr;
};
