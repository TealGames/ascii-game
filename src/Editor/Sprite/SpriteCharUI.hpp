#pragma once
#include "Core/Visual/TextBuffer.hpp"
#include "ECS/Component/Types/UI/UIInputFieldComponent.hpp"

namespace Engine::UI { class UIInteractionManager; }
namespace Engine::Input { class InputManager; }
namespace Engine::Editor::UI
{
	namespace MainUI = Engine::UI;
	class SpriteCharUI
	{
	private:
		MainUI::UIInputFieldComponent m_charField;

	public:

	private:
	public:
		SpriteCharUI(Input::InputManager& inputManager);

		void Update();
		//RenderInfo Render(const RenderInfo& renderInfo) override;

		Rendering::TextBufferCharPosition2D GetAsTextBufferChar() const;
	};
}


