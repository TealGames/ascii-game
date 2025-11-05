#include "Core/Asset/MaterialAsset.hpp"
#include "Fig/Fig.hpp"
#include "Fig/FigDeserializers.hpp"	

static const char* PROPERTY_MARKER = "Properties";

MaterialAsset::MaterialAsset(const std::filesystem::path& path) : Asset(path, false), m_material{}
{
	m_material.m_Name = GetName();
	UpdateAssetFromFile();
}

const Rendering::Material& MaterialAsset::GetMaterial() const { return m_material;  }
Rendering::Material& MaterialAsset::GetMaterialMutable() { return m_material; }

Rendering::Texture* MaterialAsset::ToTexture(const std::string& value)
{
	if (value == Fig::NULL_VALUE)
		return nullptr;

	//TODO: implement by getting asset manager and finding texture at the path and/or name
	return nullptr;
}
void MaterialAsset::UpdateAssetFromFile()
{
	Fig fig = Fig(GetAbsolutePathCopy(), FigFlag::IncludeOverflowLineStartSpaces);
	std::vector<FigPropertyRef> figProperties = {};
	fig.GetAllProperties(PROPERTY_MARKER, figProperties);

	m_material.m_Albedo = ToTexture(figProperties[0].GetValue().front());
	m_material.m_BaseColor = ToColor(figProperties[1].GetValue().front());
	m_material.m_Alpha = std::clamp(ToFloat(figProperties[2].GetValue().front()), 0.0f, float(MAX_INT_COLOR_CHANNEL));
	if (m_material.m_Alpha > 1.0f) m_material.m_Alpha /= MAX_INT_COLOR_CHANNEL;

	m_material.m_EmissiveColor = ToColor(figProperties[3].GetValue().front());
	m_material.m_Roughness = std::clamp(ToFloat(figProperties[4].GetValue().front()), 0.0f, 1.0f);
	m_material.m_Metallic = std::clamp(ToFloat(figProperties[5].GetValue().front()), 0.0f, 1.0f);
	m_material.m_NormalMap = ToTexture(figProperties[6].GetValue().front());
}

bool HasMaterialExtension(const std::string& extension)
{
	return extension == ".mater";
}