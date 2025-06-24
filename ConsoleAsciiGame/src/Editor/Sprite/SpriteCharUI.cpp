#include "pch.hpp"
#include "SpriteCharUI.hpp"
#include "EditorStyles.hpp"

SpriteCharUI::SpriteCharUI(Input::InputManager& inputManager) 
	: m_charField(inputManager, InputFieldType::String, InputFieldFlag::None, EditorStyles::GetInputFieldStyle(TextAlignment::Center))
{
}

void SpriteCharUI::Update()
{
	//m_charField.Update();
}

/*
RenderInfo SpriteCharGUI::Render(const RenderInfo& renderInfo)
{
	//TODO: implement
	return {};
}
*/

TextBufferCharPosition SpriteCharUI::GetAsTextBufferChar() const
{
	return TextBufferCharPosition();
}
