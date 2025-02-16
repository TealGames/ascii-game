//NOT USED
#include "pch.hpp"
#include "InputData.hpp"	
#include "Debug.hpp"
#include "Entity.hpp"
#include "Debug.hpp"

InputData::InputData() : ComponentData(), 
	m_actions() {}

void InputData::AddAction(const Input::InputAction& action)
{
	if (!Assert(this, m_actions.find(action.m_Name) != m_actions.end(), std::format("Tried to add the input action: {}"
		" to input data of entity: '{}', but that action already exists for this input data!",
		action.m_Name, GetEntitySafe().m_Name)))
		return;

	m_actions.emplace(action.m_Name, action);
}
const Input::InputAction* InputData::TryGetAction(const std::string & name) const
{
	auto it = m_actions.find(name);
	if (it != m_actions.end()) return &(it->second);

	return nullptr;
}
const std::unordered_map<std::string, Input::InputAction>& InputData::GetActions() const
{
	return m_actions;
}

//void InputData::SetInputManager(Input::InputManager& manager)
//{
//	m_inputManager = &manager;
//}
//const Input::InputManager& InputData::GetInputManager() const
//{
//	if (!Assert(this, m_inputManager != nullptr, std::format("Tried to get input manager from "
//		"input data of entity: '{}' but it has not been set yet by input system", GetEntitySafe().m_Name)))
//		throw std::invalid_argument("Invalid input manager state");
//
//	return *m_inputManager;
//}