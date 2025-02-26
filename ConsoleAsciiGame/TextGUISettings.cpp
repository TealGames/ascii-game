#include "pch.hpp"
#include "TextGUISettings.hpp"
#include "Point2D.hpp"

TextGUISettings::TextGUISettings(const Color& color, const float& fontSize, const float& factor,
	const TextGUIFontSize& sizeType, const int& rightIndent) :
	m_TextColor(color), m_FontSize(fontSize), m_FontSizeParentAreaFactor(factor), m_FontSizeType(sizeType), m_RightIndent(rightIndent)
{}

TextGUISettings::TextGUISettings() : TextGUISettings({}, 0, 1, TextGUIFontSize::Static, 0) {}

TextGUISettings::TextGUISettings(const Color& textColor, const TextGUIFontSize& fontSizeType, const float& fontSize, const int& rightIndent) :
	TextGUISettings(textColor, fontSizeType==TextGUIFontSize::Static? fontSize : 0, 
	fontSizeType == TextGUIFontSize::ParentArea? fontSize : 1, fontSizeType, rightIndent) {}

float TextGUISettings::GetFontSizeFromArea(const ScreenPosition& parentArea) const
{
	return static_cast<float>(parentArea.m_Y) * m_FontSizeParentAreaFactor;
}

float TextGUISettings::GetFontSize(const ScreenPosition& parentArea) const
{
	if (m_FontSizeType == TextGUIFontSize::ParentArea) return GetFontSizeFromArea(parentArea);
	return m_FontSize;
}