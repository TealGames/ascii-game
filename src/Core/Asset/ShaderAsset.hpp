#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Shader/Shader.hpp"

class ShaderAsset : public Asset
{
private:
	Rendering::Shader m_shader;
public:
	//TODO: this is not a very cross-platform approach, should be changed
	//to maybe read any file with a custom .shd extension
	static constexpr char const* EXTENSION = ".glsl";

private:
	void ReadShaderFromSingleFile();
public:
	ShaderAsset(const std::filesystem::path& path);

	const Rendering::Shader& GetShader() const;
	void UpdateAssetFromFile() override;
};
