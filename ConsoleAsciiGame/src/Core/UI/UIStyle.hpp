#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"
#include "UITextStyle.hpp"

struct UIStyle
{
	Color m_BackgroundColor;
	Color m_SecondaryColor;
	TextUIStyle m_TextSettings;

	UIStyle();
	UIStyle(const Color& backgroundColor, const TextUIStyle& textSettings);
	UIStyle(const Color& backgroundColor, const Color& secondaryColor, const TextUIStyle& textSettings);
};

