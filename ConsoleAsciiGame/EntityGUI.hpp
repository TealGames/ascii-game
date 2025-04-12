#pragma once
#include <vector>	
#include "IRenderable.hpp"
#include "InputManager.hpp"
#include "Entity.hpp"
#include "ComponentGUI.hpp"

class GUISelectorManager;

class EntityGUI : IRenderable
{
private:
	const Input::InputManager* m_inputManager;
	ECS::Entity* m_entity;

	std::vector<ComponentGUI> m_componentGUIs;
	TextGUI m_entityNameText;
public:

private:
public:
	EntityGUI(const Input::InputManager& manager, GUISelectorManager& selector, ECS::Entity& entity);

	void Update();
	void SetComponentsToStored();
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	const ECS::Entity& GetEntity() const;
};

