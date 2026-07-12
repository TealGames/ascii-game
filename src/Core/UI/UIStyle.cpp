#include "pch.hpp"
#include "Core/UI/UIStyle.hpp"

namespace Engine::UI
{
	UIStyle::UIStyle() : UIStyle(COLOR_GRAY, TextUIStyle()) {}
	UIStyle::UIStyle(const ColHDR4& backgroundColor, const TextUIStyle& textSetting) :
		UIStyle(backgroundColor, COLOR_WHITE, textSetting) {}

	UIStyle::UIStyle(const ColHDR4& backgroundColor, const ColHDR4& secondaryColor, const TextUIStyle& textSettings)
		: m_BackgroundColor(backgroundColor), m_SecondaryColor(secondaryColor), m_TextSettings(textSettings) {}
}
