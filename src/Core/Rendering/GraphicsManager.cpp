#include "Core/Rendering/GraphicsManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "Core/Asset/MaterialAsset.hpp"
#include "Core/Asset/Model3dAsset.hpp"
#include "RenderingBackend.hpp"

namespace Engine::Rendering
{
	static const std::filesystem::path SHADERS_FOLDER = "shaders";
	static const std::filesystem::path MATERIALS_FOLDER = "materials";

	static const char* DEFAULT_ALBEDO_PATH = "textures/base_albedo.png";
	static const char* DEFAULT_MATERIAL_PATH = "materials/basic/default.mater";

	static const char* BASIC_MESH_PATHS[] = { BASIC_MESH_ASSET_DIR "cube" BASIC_MESH_EXTENSION, 
											  BASIC_MESH_ASSET_DIR "sphere" BASIC_MESH_EXTENSION,
											  BASIC_MESH_ASSET_DIR "plane" BASIC_MESH_EXTENSION };

	GraphicsManager::GraphicsManager(AssetManager& assetManager)
		: m_assetManager(&assetManager), m_defaultAlbedo(nullptr), m_defaultMaterial(nullptr), m_shaders(), m_materials(), m_basicMeshes(),
		m_skybox(), m_shaderBlockBuffers(), m_shaderGlobalDefines({}), m_singleUniformShaderMap() {}

	void GraphicsManager::InitGraphicResources()
	{
		/*m_defaultShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(DEFAULT_SHADER_NAME);
		m_textureShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(TEXTURE_SHADER_NAME);
		m_forwardRenderShader = m_assetManager->TryGetTypeAssetFromLiteralMutable<ShaderAsset>(FORWAR_RENDER_SHADER_NAME);*/

		for (auto& shaderAsset : m_assetManager->GetAssetsOfTypeMutable<ShaderAsset>(SHADERS_FOLDER))
		{
			Shader& shader = shaderAsset->GetShaderMutable();
			m_shaders.emplace(std::string_view(shaderAsset->GetName()), shaderAsset);

#ifdef SKIP_COMPUTE_SHADER_INIT
			//TODO: fix error in glLinkProgram part of compute shader creation
			if (shader->GetShaderMutable().GetProgramType() == ShaderProgramType::Compute)
			{
				
				LogWarning("Compute shader asset creation is not supported");
				continue;
			}
#endif

			//NOTE: we can do this and create defines locally because program is created before it gets out of scope and since 
			//this points to memory that outlasts this scope it should be fine even if drivers do not make copies of the arguments
			std::string_view* globalDefinesViewPtr = (std::string_view*)alloca(sizeof(std::string_view) * m_shaderGlobalDefines.size());
			for (int i = 0; i < m_shaderGlobalDefines.size(); i++)
			{
				globalDefinesViewPtr[i] = m_shaderGlobalDefines[i].ToStringView();
			}

			//NOTE: we must compile program before we init buffers to ensure that when the buffer
			//has data filled from shader, shader is valid
			if (!shader.TryCreateProgram(ShaderSourceDefines{ m_shaderGlobalDefines.empty() ? 
				nullptr : globalDefinesViewPtr, m_shaderGlobalDefines.size()}))
			{
				LogError(std::format("Attempted to load all shaders and textures, but shader: {} "
					"failed to create program", shader.ToString()));
			}

			// ------------------------------------------------------------------------------
			// INIT SHADER BUFFERS (ssbo and uniform) AND CACHE ALL SINGLE UNIFORMS
			// ------------------------------------------------------------------------------
			auto bufferIt = m_shaderBlockBuffers.end();
			for (const auto& globalVarInfo : shader.GetAllGlobalVarInfo())
			{
				//If the global var is not bindable buffer (so either single or array uniform)
				//then we add it to the list of single global uniforms so that we can set all shader uniforms at once
				if (!IsShaderGlobalVarBoundableBuffer(globalVarInfo.second.m_Type))
				{
					std::string_view globalVarNameView = (std::string_view)globalVarInfo.first;
					auto uniformIt = m_singleUniformShaderMap.find(globalVarNameView);
					if (uniformIt == m_singleUniformShaderMap.end())
					{
						m_singleUniformShaderMap.emplace(globalVarNameView, std::vector<ShaderAsset*>{ shaderAsset });
					}
					else uniformIt->second.emplace_back(shaderAsset);
					continue;
				}

				bufferIt = m_shaderBlockBuffers.find(globalVarInfo.first);
				if (bufferIt == m_shaderBlockBuffers.end())
				{
					LogError(std::format("Attempted to init shader buffers for shader:{} "
						"but buffer of type:{} named:{} was not found", shader.ToString(),
						ToString(globalVarInfo.second.m_Type), globalVarInfo.first));
					return;
				}

				//If the buffer has not been allocated yet IT IS CRITICAL WE DO THIS BEFORE BINDING
				//to ensure the id is valid and is properly linked to shader
				if (!bufferIt->second->IsAllocated())
				{
					bufferIt->second->AllocateFromShaderBlock(shader);
				}

				ShaderBufferType bufferType = ShaderBufferType::Uniform;
				if (globalVarInfo.second.m_Type == ShaderGlobalVarType::StorageBuffer)
					bufferType = ShaderBufferType::Storage;

				if (!shader.TryBindBufferBlock(bufferType, bufferIt->first.GetMemPointer(), bufferIt->second->GetBindIndex()))
				{
					LogError(std::format("Attempted to init shader buffers for shader:{}"
						"but failed to bind buffer block of type:{} named:{}", shader.ToString(), 
						bufferType== ShaderBufferType::Uniform? "Uniform" : "Storage", globalVarInfo.first));
					return;
				}

				/*for (const auto& t : m_uniformBuffers)
					LogWarning(std::format("Hash for:{} is:{}", t.first, std::hash<String16>{}(t.first)));
				LogWarning(std::format("Info hash:{}", std::hash<String16>{}(uniformInfo.first)));*/
			}
		}

		for (auto& materialAsset : m_assetManager->GetAssetsOfTypeMutable<MaterialAsset>(MATERIALS_FOLDER))
		{
			//NOTE: this is very important: we have a view into the ASSET'S NAME meaning that 
			//even if the map reallocates elsewhere, it will copy the char pointer and size which will
			//NEVER change (since assets are heap allocated and never move)
			m_materials.emplace(std::string_view(materialAsset->GetName()), materialAsset);

			if (m_defaultMaterial == nullptr && materialAsset->AbsolutePathEndsWith(DEFAULT_MATERIAL_PATH))
				m_defaultMaterial = materialAsset;
		}

		for (size_t i = 0; i < sizeof(BASIC_MESH_PATHS) / sizeof(char*); i++)
		{
			m_basicMeshes.emplace(static_cast<BasicMeshType>(i),
				m_assetManager->TryGetTypeAssetFromPathMutable<Model3dAsset>(BASIC_MESH_PATHS[i]));
		}

		m_defaultAlbedo= m_assetManager->TryGetTypeAssetFromPathMutable<TextureAsset>(DEFAULT_ALBEDO_PATH);
		if (m_defaultAlbedo == nullptr)
		{
			LogError(std::format("Failed to load default albedo at path:{}", DEFAULT_ALBEDO_PATH));
		}
		//LogError(ToStringLoadedResources());
	}

	const Texture* GraphicsManager::GetDefaultAlbedo() const
	{
		return &m_defaultAlbedo->GetTexture();
	}
	Texture* GraphicsManager::GetDefaultAlbedoMutable()
	{
		return &m_defaultAlbedo->GetTextureMutable();
	}
	const Material* GraphicsManager::GetDefaultMaterial() const
	{
		return &(m_defaultMaterial->GetMaterial());
	}
	Material* GraphicsManager::GetDefaultMaterialMutable()
	{
		return &(m_defaultMaterial->GetMaterialMutable());
	}
	MaterialAsset* GraphicsManager::GetDefaultMaterialAssetMutable()
	{
		return m_defaultMaterial;
	}

	bool GraphicsManager::TrySetSkybox(const std::filesystem::path& assetPath)
	{
		m_skybox = m_assetManager->TryGetTypeAssetFromPathMutable<TextureAsset>(assetPath);
		return m_skybox != nullptr;
	}
	const Texture* GraphicsManager::GetSkybox() const
	{
		if (m_skybox == nullptr)
			return nullptr;
		return &(m_skybox->GetTexture());
	}
	Texture* GraphicsManager::GetSkyboxMutable()
	{
		if (m_skybox == nullptr)
			return nullptr;
		return &(m_skybox->GetTextureMutable());
	}

	const Shader* GraphicsManager::TryGetShader(const std::string& name) const
	{
		auto it = m_shaders.find(name.c_str());
		if (it == m_shaders.cend()) 
			return nullptr;

		if (it->second == nullptr)
			return nullptr;

		return &(it->second->GetShader());
	}
	Shader* GraphicsManager::TryGetShaderMutable(const std::string& name)
	{
		auto it = m_shaders.find(name.c_str());
		if (it == m_shaders.end()) 
			return nullptr;

		if (it->second == nullptr)
			return nullptr;

		return &(it->second->GetShaderMutable());
	}

	const Model3d* GraphicsManager::TryGetBasicMesh(const BasicMeshType mesh) const
	{
		auto it = m_basicMeshes.find(mesh);
		if (it == m_basicMeshes.end()) 
			return nullptr;

		if (it->second == nullptr)
			return nullptr;

		return &(it->second->GetModel());
	}
	Model3d* GraphicsManager::TryGetBasicMeshMutable(const BasicMeshType mesh)
	{
		Model3dAsset* asset = TryGetBasicMeshAssetMutable(mesh);
		if (asset == nullptr)
			return nullptr;
		return &(asset->GetModelMutable());
	}
	Model3dAsset* GraphicsManager::TryGetBasicMeshAssetMutable(const BasicMeshType mesh)
	{
		auto it = m_basicMeshes.find(mesh);
		if (it == m_basicMeshes.end()) return nullptr;
		return it->second;
	}
	Material* GraphicsManager::TryCreateRuntimeMaterial(const Material& material)
	{
		auto existingIt = m_runtimeMaterials.find(material.m_Name);
		if (existingIt != m_runtimeMaterials.end())
		{
			LogError(std::format("Attempted to create runtime material:{} but one with that name already exists", 
				material.ToString()));
		}
		return &(m_runtimeMaterials.emplace(material.m_Name, material).first->second);
	}
	const Material* GraphicsManager::TryGetMaterial(const std::string& name) const
	{
		auto it = m_materials.find(name.c_str());
		if (it != m_materials.end() && it->second != nullptr)
			return &(it->second->GetMaterial());

		if (m_runtimeMaterials.size() > 0)
		{
			auto runtimeIt = m_runtimeMaterials.find(String16(name));
			if (runtimeIt != m_runtimeMaterials.end())
				return &(runtimeIt->second);
		}
		return nullptr;
	}
	Material* GraphicsManager::TryGetMaterialMutable(const std::string& name)
	{
		MaterialAsset* materialAsset = TryGetMaterialAssetMutable(name);
		if (materialAsset != nullptr)
			return &(materialAsset->GetMaterialMutable());

		if (m_runtimeMaterials.size() > 0)
		{
			auto runtimeIt = m_runtimeMaterials.find(String16(name));
			if (runtimeIt != m_runtimeMaterials.end())
				return &(runtimeIt->second);
		}
		return nullptr;
	}
	MaterialAsset* GraphicsManager::TryGetMaterialAssetMutable(const std::string& name)
	{
		auto it = m_materials.find(name.c_str());
		if (it != m_materials.end())
			return it->second;
		return nullptr;
	}
	void GraphicsManager::ExecuteOnAllMaterials(const std::function<void(std::string_view, const Material&)>& action)
	{
		for (const auto& materialAssetPair : m_materials)
		{
			if (materialAssetPair.second == nullptr)
				continue;

			action(materialAssetPair.first, materialAssetPair.second->GetMaterial());
		}
		for (const auto& material : m_runtimeMaterials)
		{
			action(material.second.m_Name, material.second);
		}
	}

	void GraphicsManager::AddShaderGlobalDefine(const std::string_view& view)
	{
		m_shaderGlobalDefines.push_back(view);
	}
	void GraphicsManager::AddShaderBuffer(ShaderBuffer* buffer)
	{
		//m_bufferController.AddShaderBuffer(buffer);
		m_shaderBlockBuffers.emplace(buffer->GetName(), buffer);
	}
	//VertexLayoutBindIndex GraphicsManager::AddVertexBuffer(VertexBuffer* vertex, IndexBuffer* index)
	//{
	//	return m_bufferController.AddVertexBuffer(vertex, index);
	//}
	bool GraphicsManager::HasShaderBuffer(const std::string_view& name) const
	{
		return m_shaderBlockBuffers.find(name) != m_shaderBlockBuffers.end();
	}

	void GraphicsManager::SetUniform(const UniformDataType type, const std::string_view& name, const void* dataPtr)
	{
		auto uniformIt = m_singleUniformShaderMap.find(name);
		if (uniformIt == m_singleUniformShaderMap.end())
			return;

		for (auto& shaderAsset : uniformIt->second)
		{
			if (shaderAsset == nullptr)
				continue;

			if (!shaderAsset->GetShaderMutable().TrySetUniform(type, name.data(), dataPtr))
			{
				LogError(std::format("Attempted to set uniform named: {} globally "
					"for all shaders but failed for shader:{}", name, shaderAsset->GetShader().ToString()));
			}
		}
	}
	void GraphicsManager::SetUniformArray(const UniformDataType type, const std::string_view& name, const void* dataPtr, const size_t elements)
	{
		auto uniformIt = m_singleUniformShaderMap.find(name);
		if (uniformIt == m_singleUniformShaderMap.end())
			return;

		for (auto& shaderAsset : uniformIt->second)
		{

			if (shaderAsset == nullptr)
				continue;

			if (!shaderAsset->GetShaderMutable().TrySetUniformArray(type, name.data(), dataPtr, elements))
			{
				LogError(std::format("Attempted to set uniform array named: {} globally "
					"for all shaders but failed for shader:{}", name, shaderAsset->GetShader().ToString()));
			}
		}
	}

	std::string GraphicsManager::ToStringLoadedResources() const
	{
		std::string result = "GRAPHIC MANAGER CACHED RESOURCES: \n";
		result += std::format("SHADERS (COUNT:{}):\n", m_shaders.size());
		for (auto& shader : m_shaders)
			result += shader.second->ToString() + "\n";

		for (const auto& uniformShaders : m_singleUniformShaderMap)
		{
			result += std::format("-----------------------------------------------------------------\n"
				"Shaders with Single/Array Uniform '{}' ({}): \n"
				"------------------------------------------------------------------------------------\n", 
				uniformShaders.first, uniformShaders.second.size());
			for (const auto& shader : uniformShaders.second)
			{
				result += std::format("{}\n", shader->ToString());
			}
		}

		return result;
	}
}