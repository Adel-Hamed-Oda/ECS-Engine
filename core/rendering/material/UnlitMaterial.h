#pragma once

#include "core/private_imports.h"
#include "WorldspaceMaterial.h"
#include "core/rendering/texture/Texture.h"
#include "core/asset_management/FileReader.h"

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

		std::string vertCode = FileReader::ReadFile("assets/Unlit.vert");
		if (vertCode.empty()) vertCode = FileReader::ReadFile("../assets/Unlit.vert");
		if (vertCode.empty()) vertCode = FileReader::ReadFile("../../assets/Unlit.vert");

		std::string fragCode = FileReader::ReadFile("assets/Unlit.frag");
		if (fragCode.empty()) fragCode = FileReader::ReadFile("../assets/Unlit.frag");
		if (fragCode.empty()) fragCode = FileReader::ReadFile("../../assets/Unlit.frag");

		if (!vertCode.empty() && !fragCode.empty())
		{
			defaultShader = Shader::CreateFromSource(vertCode, fragCode);
			if (defaultShader && defaultShader->IsValid())
			{
				return defaultShader;
			}
		}

		const std::string fallbackVert = R"(
			#version 330 core
			layout (location = 0) in vec3 aPos;
			layout (location = 1) in vec3 aNormal;
			layout (location = 2) in vec4 aColor;
			layout (location = 3) in vec2 aUV;

			uniform mat4 model;
			uniform mat4 view;
			uniform mat4 projection;

			out vec4 FragVertexColor;
			out vec2 FragTextureCoordinate;

			void main()
			{
				gl_Position = projection * view * model * vec4(aPos, 1.0);
				FragVertexColor = aColor;
				FragTextureCoordinate = aUV;
			}
		)";

		const std::string fallbackFrag = R"(
			#version 330 core
			in vec4 FragVertexColor;
			in vec2 FragTextureCoordinate;

			struct Material {
				bool hasTexture;
				sampler2D textureSampler;
				vec4 baseColor;
			};
			uniform Material material;

			uniform bool hasTexture;
			uniform sampler2D textureSampler;
			uniform vec4 color;

			out vec4 FragColor;

			void main()
			{
				bool useTex = material.hasTexture || hasTexture;
				vec4 base = (material.baseColor != vec4(0.0)) ? material.baseColor : ((color != vec4(0.0)) ? color : vec4(1.0));
				vec4 result = FragVertexColor * base;
				if (useTex)
				{
					sampler2D activeSampler = material.hasTexture ? material.textureSampler : textureSampler;
					result *= texture(activeSampler, FragTextureCoordinate);
				}
				FragColor = result;
			}
		)";

		defaultShader = Shader::CreateFromSource(fallbackVert, fallbackFrag);
		return defaultShader;
	}

private:
	inline static std::shared_ptr<Shader> defaultShader = nullptr;
};
