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
	m_material.SetBaseColor(ToColor(figProperties[1].GetValue().front()));
	m_material.SetAlpha(ToFloat(figProperties[2].GetValue().front()));

	m_material.SetEmissiveColor(ToColor(figProperties[3].GetValue().front()));
	m_material.SetRoughness(ToFloat(figProperties[4].GetValue().front()));
	m_material.SetMetallic(ToFloat(figProperties[5].GetValue().front()));
	m_material.m_NormalMap = ToTexture(figProperties[6].GetValue().front());
}

bool HasMaterialExtension(const std::string& extension)
{
	return extension == ".mater";
}