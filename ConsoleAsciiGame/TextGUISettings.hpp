#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"

enum class TextGUIFontSize
{
	Static,
	ParentArea,
};

class TextGUISettings
{
public:
	Color m_TextColor;
	float m_FontSize;
	float m_FontSizeParentAreaFactor;
	TextGUIFontSize m_FontSizeType;

	int m_RightIndent;

private:
	TextGUISettings(const Color& color, const float& fontSize, const float& factor, const TextGUIFontSize& sizeType, const int& rightIndent);

public:
	TextGUISettings();
	TextGUISettings(const Color& textColor, const TextGUIFontSize& fontSizeType, const float& fontSize, const int& rightIndent);

	float GetFontSizeFromArea(const ScreenPosition& parentArea) const;
	float GetFontSize(const ScreenPosition& parentArea) const;
};

