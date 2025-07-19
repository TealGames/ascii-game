#include "pch.hpp"
#include "Core/Rendering/FontData.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Core/Asset/FontAsset.hpp"


ScreenFontProperties::ScreenFontProperties(const float fontSize, const float spacing, const FontAsset* font) 
	: m_FontAsset(font), m_Tracking(spacing), m_Size(fontSize) {}

ScreenFontProperties::ScreenFontProperties() : ScreenFontProperties(0, 0, nullptr) {}
ScreenFontProperties::ScreenFontProperties(const float fontSize, const float spacing, const FontAsset& font) 
	: ScreenFontProperties(fontSize, spacing, &font) {}

bool ScreenFontProperties::HasValidFont() const
{
	if (m_FontAsset == nullptr) return false;
	return m_FontAsset->HasValidFont();
}

WorldFontProperties::WorldFontProperties(const Vec2& rectArea, const float spacing, const FontAsset* font)
	: m_FontAsset(font), m_Tracking(spacing), m_RectSize(rectArea) {}

WorldFontProperties::WorldFontProperties() : WorldFontProperties(Vec2{}, 0, nullptr) {}
WorldFontProperties::WorldFontProperties(const Vec2& rectArea, const float spacing, const FontAsset& font)
	: WorldFontProperties(rectArea, spacing, &font) {}

bool WorldFontProperties::HasValidFont() const
{
	if (m_FontAsset == nullptr) return false;
	return m_FontAsset->HasValidFont();
}