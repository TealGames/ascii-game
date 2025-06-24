#include "pch.hpp"
#include "InputAction.hpp"
#include "InputManager.hpp"
#include "HelperFunctions.hpp"

namespace Input
{
	InputAction::InputAction(const std::string& name, const std::vector<const InputKey*>& keys) :
		m_Name(name), m_Keys(keys) {}

	bool InputAction::IsDown() const
	{
		if (m_Keys.empty()) return false;

		for (const auto& key : m_Keys)
		{
			if (key->GetState().IsDown()) return true;
		}
		return false;
	}

	bool InputAction::IsPressed() const
	{
		if (m_Keys.empty()) return false;

		for (const auto& key : m_Keys)
		{
			if (key->GetState().IsPressed()) return true;
		}
		return false;
	}

	bool InputAction::IsReleased() const
	{
		if (m_Keys.empty()) return false;

		for (const auto& key : m_Keys)
		{
			if (key->GetState().IsReleased()) return true;
		}
		return false;
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
