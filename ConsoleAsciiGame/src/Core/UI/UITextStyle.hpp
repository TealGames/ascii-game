#pragma once
#include "raylib.h"
#include "Core/Rendering/FontData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Core/UI/UIPadding.hpp"

class TextUIStyle
{
public:
	Color m_TextColor;
	TextAlignment m_TextAlignment;
	FontProperties m_FontData;
	UIPadding m_Padding;
	float m_FontSizeFactor;
	bool m_FitToArea;

public:
	TextUIStyle();
	TextUIStyle(const Color& textColor, const FontProperties& fontData, const TextAlignment& alignment=UITextComponent::DEFAULT_ALIGNMENT, 
		const UIPadding& padding= UITextComponent::DEFAULT_PADDING, const float& factor= UITextComponent::NULL_FONT_FACTOR,
		const bool& fitToArea= UITextComponent::DEFAULT_FIT_TO_AREA);
};

 