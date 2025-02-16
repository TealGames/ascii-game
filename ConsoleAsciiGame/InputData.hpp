#pragma once
#include <string>
#include "raylib.h"
#include <unordered_map>
#include "ComponentData.hpp"
#include "Point2DInt.hpp"
#include "InputAction.hpp"
#include "InputManager.hpp"

class InputData : public ComponentData
{
private:
	//Input::InputManager* m_inputManager;
	std::unordered_map<std::string, Input::InputAction> m_actions;

public:
	InputData();

	void AddAction(const Input::InputAction& action);
	const Input::InputAction* TryGetAction(const std::string& name) const;
	const std::unordered_map<std::string, Input::InputAction>& GetActions() const;

	/*void SetInputManager(Input::InputManager& manager);
	const Input::InputManager& GetInputManager() const;*/
};