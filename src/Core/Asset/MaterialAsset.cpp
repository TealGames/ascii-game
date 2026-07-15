#include "Core/Asset/MaterialAsset.hpp"
#include "Core/Serialization/FigSerializers.hpp"	
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/IOHandler.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Core/Asset/TextureAsset.hpp"

namespace Engine::Rendering
{
	static constexpr const char* HEADER_PROPERTY = "Properties";
	static constexpr const char* PROPERTY_ALBEDO = "Albedo";
	static constexpr const char* PROPERTY_BASE_COLOR = "BaseColor";
	static constexpr const char* PROPERTY_ALPHA = "Alpha";
	static constexpr const char* PROPERTY_EMISSIVE_COLOR = "EmissiveColor";
	static constexpr const char* PROPERTY_ROUGHNESS = "Roughness";
	static constexpr const char* PROPERTY_METALLIC = "Metallic";
	static constexpr const char* PROPERTY_NORMAL_MAP = "NormalMap";

	const std::array<std::string_view,1> MaterialAsset::EXTENSIONS = { ".mater" };

	MaterialAsset::MaterialAsset(const std::filesystem::path& path) 
		: Asset(path), m_material{}, m_albedoAsset(), m_normalMapAsset()
	{
		ASSET_EXTENSION_CHECK

		m_material.m_Name = GetName();
		UpdateAssetFromFile();
	}

	const Rendering::Material& MaterialAsset::GetMaterial() const { return m_material; }
	Rendering::Material& MaterialAsset::GetMaterialMutable() { return m_material; }

	void MaterialAsset::UpdateAssetFromFile()
	{
		FigFormat::Fig fig = FigFormat::Fig(GetAbsolutePathCopy(), FigFormat::FigParseFlag::IncludeOverflowLineStartSpaces);
		std::unordered_map<std::string, std::vector<std::string>> figProperties = {};
		fig.GetAllProperties(HEADER_PROPERTY, figProperties);
		if (figProperties.empty())
			return;

		m_albedoAsset = FigFormat::ToAsset<Rendering::TextureAsset>(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_ALBEDO)[0]);
		if (m_albedoAsset != nullptr) m_material.m_Albedo = &m_albedoAsset->GetTextureMutable();

		m_material.SetBaseColor(FigFormat::ToColor<float, 4>(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_BASE_COLOR)[0]));
		m_material.SetAlpha(FigFormat::ToFloat(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_ALPHA)[0]));
		m_material.SetEmissiveColor(FigFormat::ToColor<float, 4>(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_EMISSIVE_COLOR)[0]));
		m_material.SetRoughness(FigFormat::ToFloat(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_ROUGHNESS)[0]));
		m_material.SetMetallic(FigFormat::ToFloat(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_METALLIC)[0]));

		m_normalMapAsset = FigFormat::ToAsset<Rendering::TextureAsset>(fig.TryGetMarkedValue(HEADER_PROPERTY, PROPERTY_NORMAL_MAP)[0]);
		if (m_normalMapAsset != nullptr) m_material.m_NormalMap = &m_normalMapAsset->GetTextureMutable();
	}
	void MaterialAsset::SaveToPath(const std::filesystem::path& path)
	{
		FigFormat::Fig fig = FigFormat::Fig();

		//if (GetName() == "DefaultMaterial") LogError("H");
		fig.AddHeaderProperty<Rendering::TextureAsset*>(HEADER_PROPERTY, PROPERTY_ALBEDO, m_albedoAsset);

		const ColHDR4& baseColor = m_material.GetBaseColor();
		const float alpha = m_material.GetAlpha();
		const ColHDR4& emissiveColor = m_material.GetEmissiveColor();
		const float roughness = m_material.GetRoughness();
		const float metallic = m_material.GetMatallic();
		fig.AddHeaderProperty<ColHDR4>(HEADER_PROPERTY, PROPERTY_BASE_COLOR, baseColor);
		fig.AddHeaderProperty<float>(HEADER_PROPERTY, PROPERTY_ALPHA, alpha);
		fig.AddHeaderProperty<ColHDR4>(HEADER_PROPERTY, PROPERTY_EMISSIVE_COLOR, baseColor);
		fig.AddHeaderProperty<float>(HEADER_PROPERTY, PROPERTY_ROUGHNESS, roughness);
		fig.AddHeaderProperty<float>(HEADER_PROPERTY, PROPERTY_METALLIC, metallic);

		fig.AddHeaderProperty<Rendering::TextureAsset*>(HEADER_PROPERTY, PROPERTY_NORMAL_MAP, m_normalMapAsset);

		fig.WriteToPath(path);
	}
}
