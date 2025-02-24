#pragma once
#include <unordered_map>
#include <string>
#include "EntityGUI.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"

using EntityGUICollection = std::unordered_map<std::string, EntityGUI>;
class EntityEditorGUI : IRenderable
{
private:
	const Input::InputManager& m_inputManager;
	const SceneManagement::SceneManager& m_sceneManager;

	RenderInfo m_currentRenderInfo;
	EntityGUICollection m_entityGUIs;
	EntityGUICollection::iterator m_selectedEntity;

public:

private:
public:
	EntityEditorGUI(const Input::InputManager& input, const SceneManagement::SceneManager& scene);

	void SetEntityGUI(ECS::Entity& entity);

	void Update();
	void TryRender();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

