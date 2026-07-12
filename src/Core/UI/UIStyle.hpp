#pragma once
#include "Core/Primitives/ScreenPosition.hpp"
#include "Core/UI/UITextStyle.hpp"

namespace Engine::UI
{
	class UIStyle
	{
	private:
	public:
		ColHDR4 m_BackgroundColor;
		ColHDR4 m_SecondaryColor;
		TextUIStyle m_TextSettings;

	private:
	public:
		UIStyle();
		UIStyle(const ColHDR4& backgroundColor, const TextUIStyle& textSettings);
		UIStyle(const ColHDR4& backgroundColor, const ColHDR4& secondaryColor, const TextUIStyle& textSettings);
	};
}


