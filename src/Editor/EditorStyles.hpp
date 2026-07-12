#pragma once
#include "pch.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Core/UI/UIStyle.hpp"
#include "Core/UI/UITextStyle.hpp"
#include "Core/Rendering/FontData.hpp"
#include "Core/Asset/FontAsset.hpp"

namespace Engine::Editor::Styles
{
	namespace MainUI = Engine::UI;

	//-------------------------------------------------------------------
	//			DEBUG STYLES
	//-------------------------------------------------------------------
	inline constexpr ColHDR4 DEBUG_TEXT_COLOR = COLOR_WHITE;
	inline constexpr ColHDR4 DEBUG_HIGHLIGHTED_TEXT_COLOR = COLOR_YELLOW;

	//-------------------------------------------------------------------
	//			EDITOR STYLES
	//-------------------------------------------------------------------
	inline constexpr ColHDR4 EDITOR_TEXT_DEFAULT_COLOR = COLOR_WHITE;
	inline constexpr ColHDR4 EDITOR_BACKGROUND_COLOR = ColHDR4(30, 30, 30, 255);
	inline constexpr ColHDR4 EDITOR_SECONDARY_BACKGROUND_COLOR = { 60, 60, 60, 255 };
	inline constexpr ColHDR4 EDITOR_SECONDARY_COLOR = COLOR_GRAY;
	inline constexpr ColHDR4 EDITOR_PRIMARY_COLOR = {100, 100, 100, 255};

	inline constexpr Vec2 EDITOR_CHAR_SPACING = { 3, 2 };
	inline constexpr float DEFAULT_TEXT_FACTOR = 0.8;

	//-------------------------------------------------------------------
	//			GUI STYLES
	//-------------------------------------------------------------------
	inline const Rendering::FontAsset& GetEditorFont()
	{
		//TODO: this should be replaced
		Rendering::Font* font = new Rendering::Font();
		return *(new Rendering::FontAsset(*font));
	}
	inline MainUI::TextUIStyle GetTextStyleFactorSize(const MainUI::TextAlignment alignment,
		const float factor = DEFAULT_TEXT_FACTOR, const ColHDR4 color= EDITOR_TEXT_DEFAULT_COLOR)
	{
		return MainUI::TextUIStyle(color, Rendering::ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
			alignment, MainUI::UIPadding(), factor);
	}
	inline MainUI::TextUIStyle GetTextStyleSetSize(const MainUI::TextAlignment alignment, 
		const float textSize, const ColHDR4 color = EDITOR_TEXT_DEFAULT_COLOR)
	{
		return MainUI::TextUIStyle(color, Rendering::ScreenFontProperties(textSize, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
			alignment, MainUI::UIPadding());
	}

	inline MainUI::UIStyle GetInputFieldStyle(const MainUI::TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return MainUI::UIStyle(EDITOR_PRIMARY_COLOR, MainUI::TextUIStyle(EDITOR_TEXT_DEFAULT_COLOR,
			Rendering::ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()), alignment, MainUI::UIPadding(), factor));
	}

	inline MainUI::UIStyle GetSliderStyle()
	{
		return MainUI::UIStyle(EDITOR_PRIMARY_COLOR, EDITOR_SECONDARY_COLOR, MainUI::TextUIStyle());
	}

	inline MainUI::UIStyle GetToggleStyle()
	{
		return MainUI::UIStyle(EDITOR_BACKGROUND_COLOR, COLOR_WHITE, MainUI::TextUIStyle());
	}

	inline MainUI::UIStyle GetButtonStyle(const MainUI::TextAlignment alignment, const float factor = DEFAULT_TEXT_FACTOR)
	{
		return MainUI::UIStyle(EDITOR_PRIMARY_COLOR, MainUI::TextUIStyle(EDITOR_TEXT_DEFAULT_COLOR, 
			Rendering::ScreenFontProperties(0, EDITOR_CHAR_SPACING.m_X, GetEditorFont()),
				alignment, MainUI::UIPadding(), factor));
	}
}
