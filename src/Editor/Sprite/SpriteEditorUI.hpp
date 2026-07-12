#pragma once
#include "ECS/Component/Types/UI/UIButtonComponent.hpp"
#include "ECS/Component/Types/UI/UIContainer.hpp"

namespace Engine::UI { class UIHierarchy; }
namespace Engine::Assets { class AssetManager; }
namespace Engine::Input { class InputManager; }
namespace Engine::Editor::UI
{
	namespace MainUI = Engine::UI;
	class SpriteEditorUI
	{
	private:
		Assets::AssetManager* m_assetManager;
		const Input::InputManager* m_inputManager;

		MainUI::UIContainer m_guiContainer;
		//ButtonGUI m_saveButton;
	public:

	private:
	public:
		SpriteEditorUI(MainUI::UIHierarchy& hierarchy, const Input::InputManager& inputManager, Assets::AssetManager& assetManager);

		void Update(const float deltaTime);
		//ScreenPosition Render(const RenderInfo& renderInfo) override;
	};
}


