#include "pch.hpp"
#include "FontData.hpp"
#include "RaylibUtils.hpp"

FontProperties::FontProperties() : FontProperties(0, 0, {}) {}

FontProperties::FontProperties(const float fontSize, const float spacing, const Font& font) :
	m_FontType(font), m_Tracking(spacing), m_Size(fontSize) {}

bool FontProperties::HasValidFont() const
{
	return RaylibUtils::IsValidFont(m_FontType);
}