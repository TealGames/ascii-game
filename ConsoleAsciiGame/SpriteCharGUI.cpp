#include "pch.hpp"
#include "SpriteCharGUI.hpp"
#include "EntityEditorGUI.hpp"

SpriteCharGUI::SpriteCharGUI(GUISelectorManager& guiSelector, Input::InputManager& inputManager) 
	: m_charField(inputManager, guiSelector, InputFieldType::String, InputFieldFlag::None, GUISettings()) 
{
	GUISettings fieldSettings = GUISettings({50, 15}, EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), 
			TextAlignment::Center, 0.8));

	m_charField.SetSettings(fieldSettings);
}

void SpriteCharGUI::Update()
{
	m_charField.Update();
}

ScreenPosition SpriteCharGUI::Render(const RenderInfo& renderInfo)
{
	//TODO: implement
	return {};
}

TextBufferCharPosition SpriteCharGUI::GetAsTextBufferChar() const
{
	return TextBufferCharPosition();
}
