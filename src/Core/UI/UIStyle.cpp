#include "pch.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Utils/Data/ColorConstants.hpp"

UIStyle::UIStyle() : UIStyle(COLOR_GRAY, TextUIStyle()) {}
UIStyle::UIStyle(const HDRColor& backgroundColor, const TextUIStyle& textSetting) :
	UIStyle(backgroundColor, COLOR_WHITE, textSetting) {}

UIStyle::UIStyle(const HDRColor& backgroundColor, const HDRColor& secondaryColor, const TextUIStyle& textSettings)
	: m_BackgroundColor(backgroundColor), m_SecondaryColor(secondaryColor), m_TextSettings(textSettings) {}