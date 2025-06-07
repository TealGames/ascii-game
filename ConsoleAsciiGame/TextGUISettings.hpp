#pragma once
#include "raylib.h"
#include "FontData.hpp"
#include "UITextComponent.hpp"
#include "GUIPadding.hpp"

class TextGUIStyle
{
public:
	Color m_TextColor;
	TextAlignment m_TextAlignment;
	FontProperties m_FontData;
	GUIPadding m_Padding;
	float m_FontSizeFactor;
	bool m_FitToArea;

public:
	TextGUIStyle();
	TextGUIStyle(const Color& textColor, const FontProperties& fontData, const TextAlignment& alignment=UITextComponent::DEFAULT_ALIGNMENT, 
		const GUIPadding& padding= UITextComponent::DEFAULT_PADDING, const float& factor= UITextComponent::NULL_FONT_FACTOR,
		const bool& fitToArea= UITextComponent::DEFAULT_FIT_TO_AREA);
};

 