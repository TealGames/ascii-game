#pragma once
#include <unordered_map>
#include <string>
#include "EntityGUI.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"

class GUISelectorManager;

using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI : IRenderable
{
private:
	const Input::InputManager& m_inputManager;
	const SceneManagement::SceneManager& m_sceneManager;
	GUISelectorManager& m_selectorManager;

	RenderInfo m_currentRenderInfo;
	EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;

public:

private:
public:
	EntityEditorGUI(const Input::InputManager& input, const SceneManagement::SceneManager& scene, GUISelectorManager& selector);

	void SetEntityGUI(ECS::Entity& entity);

	void Update();
	void TryRender();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

