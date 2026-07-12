#include "pch.hpp"
#include "Editor/Sprite/SpriteCharUI.hpp"
#include "Editor/EditorStyles.hpp"

namespace Engine::Editor::UI
{
	SpriteCharUI::SpriteCharUI(Input::InputManager& inputManager)
		: m_charField(MainUI::InputFieldType::String, MainUI::InputFieldFlag::None, Styles::GetInputFieldStyle(MainUI::TextAlignment::Center))
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

	Rendering::TextBufferCharPosition2D SpriteCharUI::GetAsTextBufferChar() const
	{
		return {};
	}
}
