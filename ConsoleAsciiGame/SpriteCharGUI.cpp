#include "pch.hpp"
#include "SpriteCharGUI.hpp"
#include "EditorStyles.hpp"

SpriteCharGUI::SpriteCharGUI(Input::InputManager& inputManager) 
	: m_charField(inputManager, InputFieldType::String, InputFieldFlag::None, EditorStyles::GetInputFieldStyle(TextAlignment::Center))
{
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
