#include "pch.hpp"
#include "Core/UI/UITextStyle.hpp"

namespace Engine::UI
{
	bool IsTopAlignment(const TextAlignment& alignment)
	{
		return static_cast<std::uint8_t>(alignment) <= 2;
	}
	bool IsMiddleHorizontalAlignment(const TextAlignment& alignment)
	{
		std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
		return 3 <= integralValue && integralValue <= 5;
	}
	bool IsBottomAlignment(const TextAlignment& alignment)
	{
		std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
		return 6 <= integralValue && integralValue <= 8;
	}

	bool IsLeftAlignment(const TextAlignment& alignment)
	{
		std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
		return integralValue == 0 || integralValue == 3 || integralValue == 6;
	}
	bool IsMiddleVerticalAlignment(const TextAlignment& alignment)
	{
		std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
		return integralValue == 1 || integralValue == 4 || integralValue == 7;
	}
	bool IsRightAlignment(const TextAlignment& alignment)
	{
		std::uint8_t integralValue = static_cast<std::uint8_t>(alignment);
		return integralValue == 2 || integralValue == 5 || integralValue == 8;
	}

	TextUIStyle::TextUIStyle(const ColHDR4& color, const Rendering::ScreenFontProperties& fontData,
		const TextAlignment& alignment, const UIPadding& padding, const float& factor, const bool& fitToArea) :
		m_TextColor(color), m_FontData(fontData), m_TextAlignment(alignment),
		m_FontSizeFactor(factor), m_FitToArea(fitToArea), m_Padding(padding)
	{}

	TextUIStyle::TextUIStyle() : TextUIStyle(ColHDR4(), Rendering::ScreenFontProperties()) {}
}
