#pragma once
//#include "raylib.h"
#include "Utils/Data/ScreenPosition.hpp"
#include "Core/UI/UITextStyle.hpp"

struct UIStyle
{
	Utils::Color m_BackgroundColor;
	Utils::Color m_SecondaryColor;
	TextUIStyle m_TextSettings;

	UIStyle();
	UIStyle(const Utils::Color& backgroundColor, const TextUIStyle& textSettings);
	UIStyle(const Utils::Color& backgroundColor, const Utils::Color& secondaryColor, const TextUIStyle& textSettings);
};

