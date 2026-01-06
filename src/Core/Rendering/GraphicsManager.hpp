#pragma once
#include "Core/Asset/ShaderAsset.hpp"
#include <filesystem>
#include <unordered_map>
#include <queue>
#include <string_view>
#include "Utils/Data/FixedString.hpp"
#include "Core/Rendering/Buffers.hpp"
#include "Core/Rendering/Model3d.hpp"

#define BASIC_MESH_EXTENSION ".vtx"

namespace AssetManagement { class AssetManager; }
class ShaderAsset;
class TextureAsset;
class MaterialAsset;

namespace Rendering
{
	class Shader;
	class Texture;
	class Material;

	class GraphicsManager
	{
	private:
		AssetManagement::AssetManager* m_assetManager;
		//BufferController m_bufferController;
		
		std::vector<String32> m_shaderGlobalDefines;
		//Contains all shaders with their names as keys
		std::unordered_map<std::string_view, Shader*> m_shaders;
		std::unordered_map<std::string_view, Material*> m_materials;
		std::unordered_map<BasicMeshType, Model3d*> m_basicMeshes;

		//Contains all registered shader buffers (both uniform and ssbo) with their names as keys
		std::unordered_map<ShaderVarNameType, ShaderBuffer*> m_shaderBlockBuffers;
		//Contains all single/array uniform names as keys and each shader that contains that uniform as values
		std::unordered_map<std::string_view, std::vector<Shader*>> m_singleUniformShaderMap;

		TextureAsset* m_defaultAlbedo;
		MaterialAsset* m_defaultMaterial;
	public:

	private:
	public:
		GraphicsManager(AssetManagement::AssetManager& assetManager);
		void InitGraphicResources();

		const Texture* GetDefaultAlbedo() const;
		Texture* GetDefaultAlbedoMutable();
		const Material* GetDefaultMaterial() const;
		Material* GetDefaultMaterialMutable();

		const Shader* TryGetShader(const std::string& name) const;
		Shader* TryGetShaderMutable(const std::string& name);

		const Model3d* TryGetBasicMesh(const BasicMeshType mesh) const;
		Model3d* TryGetBasicMeshMutable(const BasicMeshType mesh);

		//Material* CreateMaterial(const Color& baseColor, const Color& emissiveColor, const float alpha);
		const Material* TryGetMaterial(const std::string& name) const;
		Material* TryGetMaterialMutable(const std::string& name);
		void ExecuteOnAllMaterials(const std::function<void(std::string_view nameView, const Material& material)>& action);

		void AddShaderGlobalDefine(const std::string_view& view);
		void AddShaderBuffer(ShaderBuffer* buffer);
		//VertexLayoutBindIndex AddVertexBuffer(VertexBuffer* vertex, IndexBuffer* index);
		bool HasShaderBuffer(const std::string_view& name) const;

		void SetUniform(const UniformDataType type, const std::string_view& name, const void* dataPtr);
		void SetUniformArray(const UniformDataType type, const std::string_view& name, const void* dataPtr, const size_t elements);

		std::string ToStringLoadedResources() const;
	};
}