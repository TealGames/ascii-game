#include "Core/Rendering/Material.hpp"
#include <algorithm>

namespace Rendering
{
	Material::Material() : Material("", nullptr, DEFAULT_BASE_COLOR) {}
	Material::Material(const String16& name, Texture* albedo, const Color& baseColor, const float alpha,
		const Color& emissiveColor, const float roughness, const float metallic, Texture* normal) :
		m_Name(name), m_Albedo(albedo), m_baseColor(), m_alpha(0),
		m_emissiveColor(), m_roughness(0), m_metallic(0), m_NormalMap(nullptr)
	{
		SetBaseColor(baseColor);
		SetEmissiveColor(emissiveColor);
		SetAlpha(alpha);

		SetRoughness(roughness);
		SetMetallic(metallic);
	}

	void Material::SetBaseColor(const Color& color) { m_baseColor = color.GetRangeClampAsHDR(); }
	const Color& Material::GetBaseColor() const { return m_baseColor; }

	void Material::SetEmissiveColor(const Color& color) { m_emissiveColor = color.GetRangeClampAsHDR(); }
	const Color& Material::GetEmissiveColor() const { return m_emissiveColor; }

	void Material::SetAlpha(const float value) { m_alpha = std::clamp(value, 0.0f, 1.0f); }
	float Material::GetAlpha() const { return m_alpha; }

	void Material::SetRoughness(const float value) { m_roughness = std::clamp(value, MIN_ROUGHNESS, MAX_ROUGHNESS); }
	float Material::GetRoughness() const { return m_roughness; }

	void Material::SetMetallic(const float value) { m_metallic = std::clamp(value, MIN_METALLIC, MAX_METALLIC); }
	float Material::GetMatallic() const { return m_metallic; }

	void Material::SetSurface(const float roughness, const float metallic, Texture* normalMap)
	{
		SetRoughness(roughness);
		SetMetallic(metallic);
		m_NormalMap = normalMap;
	}

	std::string Material::ToString() const
	{
		return std::format("[Material Name:{} OverlayColor:{} Alpha:{} Emissive:{} Roughness:{} Metallic:{}]",
			m_Name, m_baseColor.ToString(), m_alpha, m_emissiveColor.ToString(), m_roughness, m_metallic);
	}
}
