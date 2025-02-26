#pragma once
#include "raylib.h"
#include "ScreenPosition.hpp"
#include "TextGUISettings.hpp"

struct GUISettings
{
	ScreenPosition m_Size;
	Color m_BackgroundColor;
	TextGUISettings m_TextSettings;

	GUISettings();
	GUISettings(const ScreenPosition& size, const Color& backgroundColor, const TextGUISettings& textSettinga);
};

