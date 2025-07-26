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

		std::unordered_map<std::string_view, const Shader*> m_shaders;
	public:

	private:
	public:
		GraphicsManager(AssetManagement::AssetManager& assetManager);

		void LoadAllShaders();

		const Shader* GetDefaultShader() const;
		const Shader* TryGetShader(const std::string& name) const;
	};
}