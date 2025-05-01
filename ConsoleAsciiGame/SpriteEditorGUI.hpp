#pragma once
#include "IRenderable.hpp"
#include "ButtonGUI.hpp"

class GUISelectorManager;
namespace AssetManagement { class AssetManager; }
namespace Input { class InputManager; }

class SpriteEditorGUI : public IRenderable
{
private:
	GUISelectorManager* m_selectorManager;
	AssetManagement::AssetManager* m_assetManager;
	const Input::InputManager* m_inputManager;

	ButtonGUI m_saveButton;
public:

private:
public:
	SpriteEditorGUI(GUISelectorManager& guiSelector, const Input::InputManager& inputManager, AssetManagement::AssetManager& assetManager);

	void Update(const float deltaTime);
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

