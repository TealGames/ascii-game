#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Rendering/Buffers.hpp"

//#define SKIP_COMPUTE_SHADER_INIT

namespace Rendering
{
	static const std::filesystem::path SHADERS_FOLDER = "shaders";

	static const char* DEFAULT_ALBEDO_PATH = "textures/base_albedo.png";


	GraphicsManager::GraphicsManager(AssetManagement::AssetManager& assetManager) 
		: m_assetManager(&assetManager), m_defaultAlbedo(nullptr), m_shaders(), m_shaderGlobalDefines({}) {}

	void GraphicsManager::LoadAllShadersAndTextures()
	{
		/*m_defaultShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(DEFAULT_SHADER_NAME);
		m_textureShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(TEXTURE_SHADER_NAME);
		m_forwardRenderShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(FORWAR_RENDER_SHADER_NAME);*/

		for (auto& shader : m_assetManager->GetAssetsOfTypeMutable<ShaderAsset>(SHADERS_FOLDER))
		{
			m_shaders.emplace(std::string_view(shader->GetName()), &shader->GetShaderMutable());

#ifdef SKIP_COMPUTE_SHADER_INIT
			//TODO: fix error in glLinkProgram part of compute shader creation
			if (shader->GetShaderMutable().GetProgramType() == ShaderProgramType::Compute)
			{
				
				LogWarning("Compute shader asset creation is not supported");
				continue;
			}
#endif

			//NOTE: we must compile program before we init buffers to ensure that when the buffer
			//has data filled from shader, shader is valid
			if (!shader->GetShaderMutable().TryCreateProgram(
				ShaderSourceDefines{ m_shaderGlobalDefines.empty() ? 
				nullptr : &m_shaderGlobalDefines[0], m_shaderGlobalDefines.size()}))
			{
				LogError(std::format("Attempted to load all shaders and textures, but shader: {} "
					"failed to create program", shader->ToString()));
			}

			InitShaderBuffers(shader->GetShaderMutable());
		}

		m_defaultAlbedo= m_assetManager->TryGetTypeAssetFromPathMutable<TextureAsset>(DEFAULT_ALBEDO_PATH);
		if (m_defaultAlbedo == nullptr)
		{
			LogError(std::format("Failed to load default albedo at path:{}", DEFAULT_ALBEDO_PATH));
		}
	}

	const Texture* GraphicsManager::GetDefaultAlbedo() const
	{
		return &m_defaultAlbedo->GetTexture();
	}
	Texture* GraphicsManager::GetDefaultAlbedoMutable()
	{
		return &m_defaultAlbedo->GetTextureMutable();
	}
	const Shader* GraphicsManager::TryGetShader(const std::string& name) const
	{
		auto it = m_shaders.find(name.c_str());
		if (it == m_shaders.cend()) return nullptr;
		return it->second;
	}
	Shader* GraphicsManager::TryGetShaderMutable(const std::string& name)
	{
		auto it = m_shaders.find(name.c_str());
		if (it == m_shaders.end()) return nullptr;
		return it->second;
	}

	void GraphicsManager::AddShaderGlobalDefine(const std::string_view& view)
	{
		m_shaderGlobalDefines.push_back(view);
	}

	void GraphicsManager::AddUniformBuffer(UniformBuffer& buffer)
	{
		m_uniformBuffers.emplace(buffer.GetName(), &buffer);
		//LogWarning(std::format("Added:{}", buffer.GetName()));
	}
	bool GraphicsManager::HasUniformBuffer(const std::string_view& view) const
	{
		return m_uniformBuffers.find(view) != m_uniformBuffers.end();
	}
	void GraphicsManager::InitShaderBuffers(Shader& shader)
	{
		auto bufferIt = m_uniformBuffers.end();
		for (const auto& uniformInfo : shader.GetAllUniformInfo())
		{
			if (uniformInfo.second.m_Type != UniformType::Buffer)
				continue;

			/*for (const auto& t : m_uniformBuffers)
				LogWarning(std::format("Hash for:{} is:{}", t.first, std::hash<String16>{}(t.first)));
			LogWarning(std::format("Info hash:{}", std::hash<String16>{}(uniformInfo.first)));*/

			bufferIt = m_uniformBuffers.find(uniformInfo.first);
			if (bufferIt == m_uniformBuffers.end())
			{
				LogError(std::format("Attempted to init shader buffers for shader:{} "
					"but uniform buffer:{} was not found", shader.ToString(), uniformInfo.first));
				return;
			}

			//If the buffer has not been allocated yet IT IS CRITICAL WE DO THIS BEFORE BINDING
			//to ensure the id is valid and is properly linked to shader
			if (!bufferIt->second->IsAllocated()) bufferIt->second->AllocateFromShaderUniformBlock(shader);
			if (!shader.TryBindUniformBlock(bufferIt->first.GetMemPointer(), bufferIt->second->GetBindIndex()))
			{
				LogError(std::format("Attempted to init shader buffers for shader:{}" 
					"but failed to bind uniform block:{}", shader.ToString(), uniformInfo.first));
				return;
			}
		}
	}

	void GraphicsManager::SetUniform(const UniformDataType type, const std::string_view& name, const void* dataPtr)
	{
		//TODO: ideally when we set a uniform, we store what shaders have what uniform
		for (auto& shader : m_shaders)
		{
			if (!shader.second->HasUniform(name))
				continue;

			shader.second->TrySetUniform(type, name.data(), dataPtr);
		}
	}
	void GraphicsManager::SetUniformArray(const UniformDataType type, const std::string_view& name, const void* dataPtr, const size_t elements)
	{
		for (auto& shader : m_shaders)
		{
			if (!shader.second->HasUniform(name))
				continue;

			shader.second->TrySetUniformArray(type, name.data(), dataPtr, elements);
		}
	}

	std::string GraphicsManager::ToStringLoadedResources() const
	{
		std::string result = "GRAPHIC MANAGER CACHED RESOURCES: \n";
		result += std::format("SHADERS (COUNT:{}):\n", m_shaders.size());
		for (auto& shader : m_shaders)
			result += shader.second->ToString() + "\n";

		return result;
	}
}