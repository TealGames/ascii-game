#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"
#include "TextGUISettings.hpp"

struct GUIStyle
{
	ScreenPosition m_Size;
	Color m_BackgroundColor;
	TextGUIStyle m_TextSettings;

	GUIStyle();
	GUIStyle(const ScreenPosition& size, const Color& backgroundColor, const TextGUIStyle& textSettinga);
};

