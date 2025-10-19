#pragma once
#include "Core/Asset/ShaderAsset.hpp"
#include <filesystem>
#include <unordered_map>
#include <queue>
#include <string_view>
#include "Utils/Data/FixedString.hpp"

namespace AssetManagement { class AssetManager; }
class ShaderAsset;
class TextureAsset;

namespace Rendering
{
	class Shader;
	class Texture;
	class UniformBuffer;

	class GraphicsManager
	{
	private:
		AssetManagement::AssetManager* m_assetManager;
		
		std::vector<std::string_view> m_shaderGlobalDefines;
		std::unordered_map<std::string_view, Shader*> m_shaders;
		std::unordered_map<String16, UniformBuffer*> m_uniformBuffers;

		TextureAsset* m_defaultAlbedo;
	public:

	private:
		void InitShaderBuffers(Shader& shader);
	public:
		GraphicsManager(AssetManagement::AssetManager& assetManager);

		void LoadAllShadersAndTextures();

		const Texture* GetDefaultAlbedo() const;
		Texture* GetDefaultAlbedoMutable();

		const Shader* TryGetShader(const std::string& name) const;
		Shader* TryGetShaderMutable(const std::string& name);

		void AddShaderGlobalDefine(const std::string_view& view);

		void AddUniformBuffer(UniformBuffer& buffer);
		bool HasUniformBuffer(const std::string_view& name) const;

		void SetUniform(const UniformDataType type, const std::string_view& name, const void* dataPtr);
		void SetUniformArray(const UniformDataType type, const std::string_view& name, const void* dataPtr, const size_t elements);

		std::string ToStringLoadedResources() const;
	};
}