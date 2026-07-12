#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Shader/Shader.hpp"

namespace Engine::Rendering
{
	class ShaderAsset : public Assets::Asset
	{
	private:
		Rendering::Shader m_shader;
	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
		void ReadShaderFromSingleFile(const Rendering::ShaderProgramType programType);
		void WriteToShaderFromFiles();
	public:
		ShaderAsset(const std::filesystem::path& path);

		const Rendering::Shader& GetShader() const;
		Rendering::Shader& GetShaderMutable();
		void UpdateAssetFromFile() override;
	};
}

