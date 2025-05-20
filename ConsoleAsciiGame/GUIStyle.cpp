#include "pch.hpp"
#include "GUIStyle.hpp"

GUIStyle::GUIStyle() : GUIStyle(Color(), TextGUIStyle()) {}
GUIStyle::GUIStyle(const Color& backgroundColor, const TextGUIStyle& textSetting) :
	m_BackgroundColor(backgroundColor), m_TextSettings(textSetting) {}
