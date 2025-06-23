#include "pch.hpp"
#include "UIStyle.hpp"

UIStyle::UIStyle() : UIStyle(GRAY, TextUIStyle()) {}
UIStyle::UIStyle(const Color& backgroundColor, const TextUIStyle& textSetting) :
	UIStyle(backgroundColor, WHITE, textSetting) {}

UIStyle::UIStyle(const Color& backgroundColor, const Color& secondaryColor, const TextUIStyle& textSettings)
	: m_BackgroundColor(backgroundColor), m_SecondaryColor(secondaryColor), m_TextSettings(textSettings) {}