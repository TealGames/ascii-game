#include "pch.hpp"
#include "TextGUISettings.hpp"

TextGUIStyle::TextGUIStyle(const Color& color, const FontProperties& fontData, 
	const TextAlignment& alignment, const GUIPadding& padding, const float& factor, const bool& fitToArea) :
	m_TextColor(color), m_FontData(fontData), m_TextAlignment(alignment), 
	m_FontSizeFactor(factor), m_FitToArea(fitToArea), m_Padding(padding)
{}

TextGUIStyle::TextGUIStyle() : TextGUIStyle(Color(), FontProperties()) {}