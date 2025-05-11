#include "pch.hpp"
#include "SpriteCharGUI.hpp"
#include "EntityEditorGUI.hpp"

SpriteCharGUI::SpriteCharGUI(Input::InputManager& inputManager) 
	: m_charField(inputManager, InputFieldType::String, InputFieldFlag::None, GUIStyle()) 
{
	GUIStyle fieldSettings = GUIStyle({50, 15}, EntityEditorGUI::EDITOR_SECONDARY_COLOR,
		TextGUIStyle(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()), 
			TextAlignment::Center, 0.8));

	m_charField.SetSettings(fieldSettings);
}

void SpriteCharGUI::Update()
{
	//m_charField.Update();
}

RenderInfo SpriteCharGUI::Render(const RenderInfo& renderInfo)
{
	//TODO: implement
	return {};
}

TextBufferCharPosition SpriteCharGUI::GetAsTextBufferChar() const
{
	return TextBufferCharPosition();
}
