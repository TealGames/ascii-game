#include "pch.hpp"
#include "GUISettings.hpp"

GUISettings::GUISettings() : GUISettings(ScreenPosition(), Color(), TextGUISettings()) {}
GUISettings::GUISettings(const ScreenPosition& size, const Color& backgroundColor, const TextGUISettings& textSetting) :
	m_Size(size), m_BackgroundColor(backgroundColor), m_TextSettings(textSetting) {}
