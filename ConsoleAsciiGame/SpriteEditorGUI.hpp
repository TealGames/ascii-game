#pragma once
#include "ITreeGUIConstructible.hpp"
#include "ButtonGUI.hpp"
#include "ContainerGUI.hpp"

class GUIHierarchy;
namespace AssetManagement { class AssetManager; }
namespace Input { class InputManager; }

class SpriteEditorGUI
{
private:
	AssetManagement::AssetManager* m_assetManager;
	const Input::InputManager* m_inputManager;

	ContainerGUI m_guiContainer;
	ButtonGUI m_saveButton;
public:

private:
public:
	SpriteEditorGUI(GUIHierarchy& hierarchy, const Input::InputManager& inputManager, AssetManagement::AssetManager& assetManager);

	void Update(const float deltaTime);
	//ScreenPosition Render(const RenderInfo& renderInfo) override;
};

