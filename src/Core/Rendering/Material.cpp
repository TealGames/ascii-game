#include "Core/Rendering/Material.hpp"
#include <algorithm>

namespace Engine::Rendering
{
	Material::Material() : Material("", nullptr, DEFAULT_BASE_COLOR) {}
	Material::Material(const std::string& name, Texture* albedo, const ColHDR4& baseColor, const float alpha,
		const ColHDR4& emissiveColor, const float roughness, const float metallic, Texture* normal) :
		m_Name(name), m_Albedo(albedo), m_baseColor(), m_alpha(0),
		m_emissiveColor(), m_roughness(0), m_metallic(0), m_NormalMap(nullptr)
	{
		SetBaseColor(baseColor);
		SetEmissiveColor(emissiveColor);
		SetAlpha(alpha);

		SetRoughness(roughness);
		SetMetallic(metallic);
	}

	Material& Material::SetBaseColor(const ColHDR4& color) 
	{ 
		m_baseColor = ClampRangeAsHDR(color); 
		return *this;
	}
	const ColHDR4& Material::GetBaseColor() const { return m_baseColor; }

	Material& Material::SetEmissiveColor(const ColHDR4& color)
	{
		m_emissiveColor = ClampRangeAsHDR(color);
		return *this;
	}
	const ColHDR4& Material::GetEmissiveColor() const { return m_emissiveColor; }

	Material& Material::SetAlpha(const float value)
	{
		m_alpha = std::clamp(value, 0.0f, 1.0f);
		return *this;
	}
	float Material::GetAlpha() const { return m_alpha; }

	Material& Material::SetRoughness(const float value)
	{
		m_roughness = std::clamp(value, MIN_ROUGHNESS, MAX_ROUGHNESS);
		return *this;
	}
	float Material::GetRoughness() const { return m_roughness; }

	Material& Material::SetMetallic(const float value)
	{
		m_metallic = std::clamp(value, MIN_METALLIC, MAX_METALLIC);
		return *this;
	}
	float Material::GetMatallic() const { return m_metallic; }

	Material& Material::SetSurface(const float roughness, const float metallic, Texture* normalMap)
	{
		SetRoughness(roughness);
		SetMetallic(metallic);
		m_NormalMap = normalMap;
		return *this;
	}

	std::string Material::ToString() const
	{
		return std::format("[Material Name:{} BaseColor:{} Alpha:{} Emissive:{} Roughness:{} Metallic:{}]",
			m_Name, m_baseColor.ToString(), m_alpha, m_emissiveColor.ToString(), m_roughness, m_metallic);
	}
}
