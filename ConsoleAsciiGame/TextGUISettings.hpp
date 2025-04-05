#pragma once
#include "raylib.h"
#include "FontData.hpp"
#include "TextGUI.hpp"
#include "GUIPadding.hpp"

class TextGUISettings
{
public:
	Color m_TextColor;
	TextAlignment m_TextAlignment;
	FontData m_FontData;
	GUIPadding m_Padding;
	float m_CharSpacing;
	float m_FontSizeFactor;
	bool m_FitToArea;

public:
	TextGUISettings();
	TextGUISettings(const Color& textColor, const FontData& fontData, const float& spacing, 
		const TextAlignment& alignment=TextGUI::DEFAULT_ALIGNMENT, const GUIPadding& padding= TextGUI::DEFAULT_PADDING, const float& factor= TextGUI::NULL_FONT_FACTOR,
		const bool& fitToArea= TextGUI::DEFAULT_FIT_TO_AREA);
};

 