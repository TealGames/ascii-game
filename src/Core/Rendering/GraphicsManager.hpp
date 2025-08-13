#pragma once
#include <filesystem>
#include <unordered_map>
#include <string_view>

namespace AssetManagement { class AssetManager; }
class ShaderAsset;

namespace Rendering
{
	class Shader;
	class GraphicsManager
	{
	private:
		AssetManagement::AssetManager* m_assetManager;
		ShaderAsset* m_defaultShader;
		ShaderAsset* m_textureShader;
		ShaderAsset* m_forwardRenderShader;

		std::unordered_map<std::string_view, const Shader*> m_shaders;
	public:

	private:
	public:
		GraphicsManager(AssetManagement::AssetManager& assetManager);

		void LoadAllShaders();

		const Shader* GetDefaultShader() const;
		Shader* GetDefaultShaderMutable();
		const Shader* GetTextureShader() const;
		Shader* GetTextureShaderMutable();
		const Shader* GetFowardRenderShader() const;
		Shader* GetForwardRenderShaderMutable();

		const Shader* TryGetShader(const std::string& name) const;
	};
}