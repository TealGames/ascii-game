#include "pch.hpp"
#include "GUIStyle.hpp"

GUIStyle::GUIStyle() : GUIStyle(ScreenPosition(), Color(), TextGUIStyle()) {}
GUIStyle::GUIStyle(const ScreenPosition& size, const Color& backgroundColor, const TextGUIStyle& textSetting) :
	m_Size(size), m_BackgroundColor(backgroundColor), m_TextSettings(textSetting) {}
