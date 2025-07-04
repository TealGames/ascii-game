#include "pch.hpp"
#include "Core/Rendering/FontData.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/Asset/FontAsset.hpp"


FontProperties::FontProperties(const float fontSize, const float spacing, const FontAsset* font) 
	: m_FontAsset(font), m_Tracking(spacing), m_Size(fontSize) {}

FontProperties::FontProperties() : FontProperties(0, 0, nullptr) {}
FontProperties::FontProperties(const float fontSize, const float spacing, const FontAsset& font) 
	: FontProperties(fontSize, spacing, &font) {}

bool FontProperties::HasValidFont() const
{
	if (m_FontAsset == nullptr) return false;
	return m_FontAsset->HasValidFont();
}