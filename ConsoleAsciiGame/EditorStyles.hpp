#pragma once
#include "pch.hpp"
#include "raylib.h"
#include "Vec2.hpp"
#include "GUIStyle.hpp"

namespace EditorStyles
{
	//-------------------------------------------------------------------
	//			DEBUG STYLES
	//-------------------------------------------------------------------
	inline const Color DEBUG_TEXT_COLOR = WHITE;
	inline const Color DEBUG_HIGHLIGHTED_TEXT_COLOR = YELLOW;

	//-------------------------------------------------------------------
	//			EDITOR STYLES
	//-------------------------------------------------------------------
	inline const Color EDITOR_TEXT_DEFAULT_COLOR = WHITE;
	inline const Color EDITOR_BACKGROUND_COLOR = { 30, 30, 30, 255 };
	inline const Color EDITOR_SECONDARY_BACKGROUND_COLOR = { 45, 45, 45, 255 };
	inline const Color EDITOR_SECONDARY_COLOR = GRAY;
	inline const Color EDITOR_PRIMARY_COLOR = DARKGRAY;

	inline const Vec2 EDITOR_CHAR_SPACING = { 3, 2 };
	inline constexpr float DEFAULT_TEXT_FACTOR = 0.8;

	//-------------------------------------------------------------------
	//			RENDER STYLES
	//-------------------------------------------------------------------
	inline const Color COLLIDER_OUTLINE_COLOR = GREEN;
	inline const Color LINE_COLOR = RED;
	inline const Color GUI_BOUNDS_COLOR = YELLOW;

	//-------------------------------------------------------------------
	//			GUI STYLES
	//-------------------------------------------------------------------
	inline TextGUIStyle GetTextStyle(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR, const Color color= EDITOR_TEXT_DEFAULT_COLOR)
	{
		return TextGUIStyle(color, FontProperties(0, EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
			alignment, GUIPadding(), factor);
	}

	inline GUIStyle GetInputFieldStyle(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return GUIStyle(EDITOR_SECONDARY_COLOR, TextGUIStyle(EDITOR_TEXT_DEFAULT_COLOR,
			FontProperties(0, EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), alignment, GUIPadding(), factor));
	}

	inline GUIStyle GetSliderStyle()
	{
		return GUIStyle(EDITOR_PRIMARY_COLOR, EDITOR_SECONDARY_COLOR, TextGUIStyle());
	}

	inline GUIStyle GetToggleStyle()
	{
		return GUIStyle(EDITOR_SECONDARY_COLOR, WHITE, TextGUIStyle());
	}

	inline GUIStyle GetButtonStyle(const TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return GUIStyle(EDITOR_PRIMARY_COLOR, TextGUIStyle(EDITOR_TEXT_DEFAULT_COLOR, FontProperties(0, EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
				alignment, GUIPadding(), factor));
	}
}
