#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"
#include "TextGUISettings.hpp"

struct GUIStyle
{
	Color m_BackgroundColor;
	Color m_SecondaryColor;
	TextGUIStyle m_TextSettings;

	GUIStyle();
	GUIStyle(const Color& backgroundColor, const TextGUIStyle& textSettings);
	GUIStyle(const Color& backgroundColor, const Color& secondaryColor, const TextGUIStyle& textSettings);
};

