#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/ShaderAsset.hpp"

namespace Rendering
{
	static const std::filesystem::path SHADERS_FOLDER = "shaders";
	static const const char* DEFAULT_SHADER_NAME = "default";
	static const const char* TEXTURE_SHADER_NAME = "texture";
	static const const char* FORWAR_RENDER_SHADER_NAME = "forward_render";


	GraphicsManager::GraphicsManager(AssetManagement::AssetManager& assetManager) 
		: m_assetManager(&assetManager), m_defaultShader(nullptr), m_textureShader(nullptr), m_shaders() {}

	void GraphicsManager::LoadAllShaders()
	{
		m_defaultShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(DEFAULT_SHADER_NAME);
		m_textureShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(TEXTURE_SHADER_NAME);
		m_forwardRenderShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(FORWAR_RENDER_SHADER_NAME);
		if (m_defaultShader == nullptr)
		{
			LogError(std::format("Failed to load default shader by name:{}", DEFAULT_SHADER_NAME));
			return;
		}
		for (auto& shader : m_assetManager->GetAssetsOfTypeMutable<ShaderAsset>(SHADERS_FOLDER))
		{
			m_shaders.emplace(std::string_view(shader->GetName()), &shader->GetShader());
		}
	}

	const Shader* GraphicsManager::GetDefaultShader() const
	{
		/*LogError(std::format("Getting default shader:{} vsource:{} fragsource:{}", m_defaultShader->ToString(), 
			m_defaultShader->GetShader().GetVertexSource(), m_defaultShader->GetShader().GetFragmnetSource()));*/
		return &m_defaultShader->GetShader();
	}
	Shader* GraphicsManager::GetDefaultShaderMutable()
	{
		return &m_defaultShader->GetShaderMutable();
	}
	const Shader* GraphicsManager::GetTextureShader() const
	{
		return &m_textureShader->GetShader();
	}
	Shader* GraphicsManager::GetTextureShaderMutable()
	{
		return &m_textureShader->GetShaderMutable();
	}
	const Shader* GraphicsManager::GetFowardRenderShader() const
	{
		return &m_forwardRenderShader->GetShader();
	}
	Shader* GraphicsManager::GetForwardRenderShaderMutable()
	{
		return &m_forwardRenderShader->GetShaderMutable();
	}
	const Shader* GraphicsManager::TryGetShader(const std::string& name) const
	{
		auto it = m_shaders.find(name.c_str());
		if (it == m_shaders.end()) return nullptr;
		return it->second;
	}
}