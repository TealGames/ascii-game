#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Material.hpp"

namespace Engine::Rendering
{
	class TextureAsset;
	class MaterialAsset : public Assets::Asset
	{
	private:
		Rendering::TextureAsset* m_albedoAsset;
		Rendering::TextureAsset* m_normalMapAsset;
		Rendering::Material m_material;

	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
	public:
		MaterialAsset(const std::filesystem::path& path);
		~MaterialAsset() = default;

		const Rendering::Material& GetMaterial() const;
		Rendering::Material& GetMaterialMutable();
		void UpdateAssetFromFile() override;
		void SaveToPath(const std::filesystem::path& path) override;
	};
}
