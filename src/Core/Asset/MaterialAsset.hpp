#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Material.hpp"

class MaterialAsset : public Asset
{
private:
	Rendering::Material m_material;
public:

private:
	Rendering::Texture* ToTexture(const std::string& value);
public:
	MaterialAsset(const std::filesystem::path& path);
	~MaterialAsset() = default;

	const Rendering::Material& GetMaterial() const;
	Rendering::Material& GetMaterialMutable();
	void UpdateAssetFromFile() override;
};

bool HasMaterialExtension(const std::string& extension);