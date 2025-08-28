#pragma once
#include <filesystem>
#include <unordered_map>
#include <string_view>

namespace AssetManagement { class AssetManager; }
class ShaderAsset;
class TextureAsset;

namespace Rendering
{
	class Shader;
	class Texture;
	class GraphicsManager
	{
	private:
		AssetManagement::AssetManager* m_assetManager;

		std::unordered_map<std::string_view, Shader*> m_shaders;
		TextureAsset* m_defaultAlbedo;
	public:

	private:
	public:
		GraphicsManager(AssetManagement::AssetManager& assetManager);

		void LoadAllShadersAndTextures();

		const Texture* GetDefaultAlbedo() const;
		Texture* GetDefaultAlbedoMutable();

		const Shader* TryGetShader(const std::string& name) const;
		Shader* TryGetShaderMutable(const std::string& name);
	};
}