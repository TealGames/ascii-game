#include "pch.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Utils/Data/ColorConstants.hpp"

UIStyle::UIStyle() : UIStyle(Utils::COLOR_GRAY, TextUIStyle()) {}
UIStyle::UIStyle(const Utils::Color& backgroundColor, const TextUIStyle& textSetting) :
	UIStyle(backgroundColor, Utils::COLOR_WHITE, textSetting) {}

UIStyle::UIStyle(const Utils::Color& backgroundColor, const Utils::Color& secondaryColor, const TextUIStyle& textSettings)
	: m_BackgroundColor(backgroundColor), m_SecondaryColor(secondaryColor), m_TextSettings(textSettings) {}