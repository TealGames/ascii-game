#pragma once
#include <string>
#include "raylib.h"
#include <unordered_map>
#include "Component.hpp"
#include "Point2DInt.hpp"
#include "InputAction.hpp"
#include "InputManager.hpp"

class InputData : public Component
{
private:
	//Input::InputManager* m_inputManager;
	std::unordered_map<std::string, Input::InputAction> m_actions;

public:
	InputData();

	void AddAction(const Input::InputAction& action);
	const Input::InputAction* TryGetAction(const std::string& name) const;
	const std::unordered_map<std::string, Input::InputAction>& GetActions() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	/*void SetInputManager(Input::InputManager& manager);
	const Input::InputManager& GetInputManager() const;*/
};