#include "pch.hpp"
#include "GUIStyle.hpp"

GUIStyle::GUIStyle() : GUIStyle(GRAY, TextGUIStyle()) {}
GUIStyle::GUIStyle(const Color& backgroundColor, const TextGUIStyle& textSetting) :
	GUIStyle(backgroundColor, WHITE, textSetting) {}

GUIStyle::GUIStyle(const Color& backgroundColor, const Color& secondaryColor, const TextGUIStyle& textSettings)
	: m_BackgroundColor(backgroundColor), m_SecondaryColor(secondaryColor), m_TextSettings(textSettings) {}