#pragma once
#include "ECS/Component/Types/UI/UIButton.hpp"
#include "ECS/Component/Types/UI/UIContainer.hpp"

class UIHierarchy;
namespace AssetManagement { class AssetManager; }
namespace Input { class InputManager; }

class SpriteEditorUI
{
private:
	AssetManagement::AssetManager* m_assetManager;
	const Input::InputManager* m_inputManager;

	UIContainer m_guiContainer;
	//ButtonGUI m_saveButton;
public:

private:
public:
	SpriteEditorUI(UIHierarchy& hierarchy, const Input::InputManager& inputManager, AssetManagement::AssetManager& assetManager);

	void Update(const float deltaTime);
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
};

