#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"
#include "TextGUISettings.hpp"

struct GUIStyle
{
	Color m_BackgroundColor;
	TextGUIStyle m_TextSettings;

	GUIStyle();
	GUIStyle(const Color& backgroundColor, const TextGUIStyle& textSettings);
};

