#pragma once
//#include "raylib.h"
#include "Core/Rendering/FontData.hpp"
#include "Utils/Data/Color.hpp"
#include "Core/UI/UIPadding.hpp"

enum class TextAlignment : std::uint8_t
{
	TopLeft = 0,
	TopCenter = 1,
	TopRight = 2,

	CenterLeft = 3,
	Center = 4,
	CenterRight = 5,

	BottomLeft = 6,
	BottomCenter = 7,
	BottomRight = 8,
};

/// <summary>
/// Return true if alignment is the top row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsTopAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is middle row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsMiddleHorizontalAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is bottom row in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsBottomAlignment(const TextAlignment& alignment);

/// <summary>
/// Returns true if alignment is left column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsLeftAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is middle column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsMiddleVerticalAlignment(const TextAlignment& alignment);
/// <summary>
/// Returns true if alignment is right column in 3x3 grid
/// </summary>
/// <param name="alignment"></param>
/// <returns></returns>
bool IsRightAlignment(const TextAlignment& alignment);

inline constexpr float NULL_FONT_FACTOR = 0;
inline constexpr bool DEFAULT_FIT_TO_AREA = true;
inline constexpr TextAlignment DEFAULT_ALIGNMENT = TextAlignment::Center;
inline const UIPadding DEFAULT_PADDING = UIPadding();

class TextUIStyle
{
public:
	HDRColor m_TextColor;
	TextAlignment m_TextAlignment;
	ScreenFontProperties m_FontData;
	UIPadding m_Padding;
	float m_FontSizeFactor;
	bool m_FitToArea;

public:
	TextUIStyle();
	TextUIStyle(const HDRColor& textColor, const ScreenFontProperties& fontData, const TextAlignment& alignment= DEFAULT_ALIGNMENT, 
		const UIPadding& padding= DEFAULT_PADDING, const float& factor= NULL_FONT_FACTOR,
		const bool& fitToArea= DEFAULT_FIT_TO_AREA);
};

 