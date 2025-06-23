#include "pch.hpp"
#include "UITextStyle.hpp"

TextUIStyle::TextUIStyle(const Color& color, const FontProperties& fontData, 
	const TextAlignment& alignment, const UIPadding& padding, const float& factor, const bool& fitToArea) :
	m_TextColor(color), m_FontData(fontData), m_TextAlignment(alignment), 
	m_FontSizeFactor(factor), m_FitToArea(fitToArea), m_Padding(padding)
{}

TextUIStyle::TextUIStyle() : TextUIStyle(Color(), FontProperties()) {}