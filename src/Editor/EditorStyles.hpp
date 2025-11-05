#pragma once
#include "pch.hpp"
//#include "raylib.h"
#include "Utils/Data/Vec2Type.hpp"
#include "Core/UI/UIStyle.hpp"
#include "StaticReferenceGlobals.hpp"
#include "Utils/Data/ColorConstants.hpp"

namespace EditorStyles
{
	//-------------------------------------------------------------------
	//			DEBUG STYLES
	//-------------------------------------------------------------------
	inline constexpr Color DEBUG_TEXT_COLOR = COLOR_WHITE;
	inline constexpr Color DEBUG_HIGHLIGHTED_TEXT_COLOR = COLOR_YELLOW;

	//-------------------------------------------------------------------
	//			EDITOR STYLES
	//-------------------------------------------------------------------
	inline constexpr Color EDITOR_TEXT_DEFAULT_COLOR = COLOR_WHITE;
	inline constexpr Color EDITOR_BACKGROUND_COLOR = Color(30, 30, 30, 255);
	inline constexpr Color EDITOR_SECONDARY_BACKGROUND_COLOR = { 60, 60, 60, 255 };
	inline constexpr Color EDITOR_SECONDARY_COLOR = COLOR_GRAY;
	inline constexpr Color EDITOR_PRIMARY_COLOR = {100, 100, 100, 255};

	inline constexpr Vec2 EDITOR_CHAR_SPACING = { 3, 2 };
	inline constexpr float DEFAULT_TEXT_FACTOR = 0.8;

	//-------------------------------------------------------------------
	//			GUI STYLES
	//-------------------------------------------------------------------
	inline const FontAsset& GetEditorFont()
	{
		return StaticReferenceGlobals::GetDefaultRaylibFont();
	}
	inline TextUIStyle GetTextStyleFactorSize(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR, const Color color= EDITOR_TEXT_DEFAULT_COLOR)
	{
		return TextUIStyle(color, ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
			alignment, UIPadding(), factor);
	}
	inline TextUIStyle GetTextStyleSetSize(const TextAlignment alignment, const float textSize, const Color color = EDITOR_TEXT_DEFAULT_COLOR)
	{
		return TextUIStyle(color, ScreenFontProperties(textSize, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
			alignment, UIPadding());
	}

	inline UIStyle GetInputFieldStyle(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return UIStyle(EDITOR_PRIMARY_COLOR, TextUIStyle(EDITOR_TEXT_DEFAULT_COLOR,
			ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()), alignment, UIPadding(), factor));
	}

	inline UIStyle GetSliderStyle()
	{
		return UIStyle(EDITOR_PRIMARY_COLOR, EDITOR_SECONDARY_COLOR, TextUIStyle());
	}

	inline UIStyle GetToggleStyle()
	{
		return UIStyle(EDITOR_BACKGROUND_COLOR, COLOR_WHITE, TextUIStyle());
	}

	inline UIStyle GetButtonStyle(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return UIStyle(EDITOR_PRIMARY_COLOR, TextUIStyle(EDITOR_TEXT_DEFAULT_COLOR, ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
				alignment, UIPadding(), factor));
	}
}
