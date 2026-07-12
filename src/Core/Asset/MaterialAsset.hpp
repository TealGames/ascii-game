#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Material.hpp"

namespace Engine::Rendering
{
	class MaterialAsset : public Assets::Asset
	{
	private:
		Rendering::Material m_material;
	public:
		static const std::array<std::string_view,1> EXTENSIONS;
	private:
		Rendering::Texture* ToTexture(const std::string& value);
	public:
		MaterialAsset(const std::filesystem::path& path);
		~MaterialAsset() = default;

		const Rendering::Material& GetMaterial() const;
		Rendering::Material& GetMaterialMutable();
		void UpdateAssetFromFile() override;
	};
}
