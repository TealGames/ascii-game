#include "pch.hpp"
#include "TextGUISettings.hpp"

TextGUISettings::TextGUISettings(const Color& color, const FontData& fontData, const float& spacing, 
	const TextAlignment& alignment, const GUIPadding& padding, const float& factor, const bool& fitToArea) :
	m_TextColor(color), m_FontData(fontData), m_CharSpacing(spacing), m_TextAlignment(alignment), m_FontSizeFactor(factor), m_FitToArea(fitToArea), m_Padding(padding)
{}

TextGUISettings::TextGUISettings() : TextGUISettings(Color(), FontData(), 0) {}