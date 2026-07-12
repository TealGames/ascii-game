#include "pch.hpp"
#include "Core/Input/InputAction.hpp"
#include "Core/Input/InputManager.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Engine::Input
{
	InputAction::InputAction(const std::string& name, const std::vector<const InputKeyState*>& keys) :
		m_Name(name), m_Keys(keys) {}

	bool InputAction::IsState(const KeyState state) const
	{
		if (m_Keys.empty()) return false;

		for (const auto& key : m_Keys)
		{
			if (key->GetState().GetKeyState()== state) 
				return true;
		}
		return false;
	}

	bool InputAction::IsDown() const
	{
		return IsState(KeyState::Down);
	}

	bool InputAction::IsPressed() const
	{
		return IsState(KeyState::Pressed);
	}

	bool InputAction::IsReleased() const
	{
		return IsState(KeyState::Released);
	}

	std::string InputAction::ToString() const
	{
		std::string keyStr = "";

		size_t index = 0;
		for (const auto& key : m_Keys)
		{
			if (index != 0) keyStr += ",";
			keyStr += key->ToString();

			index++;
		}
		return std::format("[{}->{}]", m_Name, keyStr);
	}
}
