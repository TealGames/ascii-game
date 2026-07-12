#include "Core/Asset/MaterialAsset.hpp"
#include "Core/Serialization/FigSerializers.hpp"	
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/IOHandler.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Core/Asset/TextureAsset.hpp"

namespace Engine::Rendering
{
	static const char* PROPERTY_MARKER = "Properties";
	const std::array<std::string_view,1> MaterialAsset::EXTENSIONS = { ".mater" };

	MaterialAsset::MaterialAsset(const std::filesystem::path& path) : Asset(path), m_material{}
	{
		ASSET_EXTENSION_CHECK

		m_material.m_Name = GetName();
		UpdateAssetFromFile();
	}

	const Rendering::Material& MaterialAsset::GetMaterial() const { return m_material; }
	Rendering::Material& MaterialAsset::GetMaterialMutable() { return m_material; }

	Rendering::Texture* MaterialAsset::ToTexture(const std::string& value)
	{
		std::string trimmedValue = Utils::StringUtil(value).TrimAnySpaceChar().ToString();
		if (FigFormat::IsNull(trimmedValue, false))
			return nullptr;

		Serialization::SerializedAsset serializedAsset = { trimmedValue };
		TextureAsset* textureAsset = Serialization::TryDeserializeTypeAsset<TextureAsset>(serializedAsset);
		ENGINE_ASSERT(textureAsset != nullptr, "Failed to deserialize Fig texture asset: {}", trimmedValue);
		return &(textureAsset->GetTextureMutable());
	}
	void MaterialAsset::UpdateAssetFromFile()
	{
		FigFormat::Fig fig = FigFormat::Fig(GetAbsolutePathCopy(), FigFormat::FigFlag::IncludeOverflowLineStartSpaces);
		std::vector<FigFormat::FigPropertyRef> figProperties = {};
		fig.GetAllProperties(PROPERTY_MARKER, figProperties);
		if (figProperties.empty())
			return;

		m_material.m_Albedo = ToTexture(figProperties[0].GetValue().front());
		m_material.SetBaseColor(Serialization::ToColor(figProperties[1].GetValue().front()));
		m_material.SetAlpha(FigFormat::ToFloat(figProperties[2].GetValue().front()));

		m_material.SetEmissiveColor(Serialization::ToColor(figProperties[3].GetValue().front()));
		m_material.SetRoughness(FigFormat::ToFloat(figProperties[4].GetValue().front()));
		m_material.SetMetallic(FigFormat::ToFloat(figProperties[5].GetValue().front()));
		m_material.m_NormalMap = ToTexture(figProperties[6].GetValue().front());
	}
}
