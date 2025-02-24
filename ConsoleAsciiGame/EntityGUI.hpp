#pragma once
#include <vector>	
#include "IRenderable.hpp"
#include "InputManager.hpp"
#include "Entity.hpp"
#include "ComponentGUI.hpp"

class EntityGUI : IRenderable
{
private:
	const Input::InputManager& m_inputManager;
	ECS::Entity& m_entity;

	std::vector<ComponentGUI> m_componentGUIs;
public:

private:
public:
	EntityGUI(const Input::InputManager& manager, ECS::Entity& entity);

	void Update();
	void SetComponentsToStored();
	ScreenPosition Render(const RenderInfo& renderInfo) override;

	const ECS::Entity& GetEntity() const;
};

