#pragma once
//#include "raylib.h"
#include "Utils/Math/ScreenPosition.hpp"
#include "Core/UI/UITextStyle.hpp"

struct UIStyle
{
	HDRColor m_BackgroundColor;
	HDRColor m_SecondaryColor;
	TextUIStyle m_TextSettings;

	UIStyle();
	UIStyle(const HDRColor& backgroundColor, const TextUIStyle& textSettings);
	UIStyle(const HDRColor& backgroundColor, const HDRColor& secondaryColor, const TextUIStyle& textSettings);
};

