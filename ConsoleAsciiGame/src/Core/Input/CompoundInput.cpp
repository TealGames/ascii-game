#include "pch.hpp"
#include "Core/Input/CompoundInput.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Core/Input/InputManager.hpp"

namespace Input
{
	CompoundInput::CompoundInput() : CompoundInput("") {}

	CompoundInput::CompoundInput(const std::string& name) :
		m_name(name), m_dirKeys() {}

	CompoundInput::CompoundInput(const std::string& name, const CompoundDirectionCollection& keys) :
		m_name(name), m_dirKeys(keys) 
	{
		//for (const auto& dir : )
	}

	const std::string& CompoundInput::GetName() const
	{
		return m_name;
	}

	const InputAction* CompoundInput::TryGetDirectionAction(const InputDirection& dir) const
	{
		auto it = m_dirKeys.find(dir);
		if (it != m_dirKeys.end()) return &(it->second);

		return nullptr;
	}
	const CompoundDirectionCollection& CompoundInput::GetEntries() const
	{
		return m_dirKeys;
	}
	std::size_t CompoundInput::GetEntriesCount() const
	{
		return m_dirKeys.size();
	}

	bool CompoundInput::HasDirection(const InputDirection& dir) const
	{
		return m_dirKeys.find(dir) != m_dirKeys.end();
	}

	void CompoundInput::AddEntry(const InputDirection& dir, const InputAction& action)
	{
		if (!Assert(!HasDirection(dir), std::format("Tried to add compound input entry of action: {} "
			"to dir: {} that already contains an input action", action.ToString(), Input::ToString(dir))))
			return;

		m_dirKeys.emplace(dir, action);
	}

	Vec2Int CompoundInput::GetCompoundInputDown() const
	{
		Vec2Int dir = {};
		for (const auto& entry : m_dirKeys)
		{
			/*LogError(std::format("Compound: {} key: {} STATE: {} (down: {})", name, std::to_string(entry.second),
				ToString(GetKeyState(entry.second)), std::to_string(IsKeyDown(entry.second))));*/
			if (!entry.second.IsDown()) continue;

			if (entry.first == InputDirection::Up) dir.m_Y++;
			else if (entry.first == InputDirection::Down) dir.m_Y--;
			else if (entry.first == InputDirection::Right) dir.m_X++;
			else if (entry.first == InputDirection::Left) dir.m_X--;
		}
		//LogError(std::format("When retrieving compound: {} -> {}", name, dir.ToString()));
		return dir;
	}
	/*std::vector<KeyState> CompoundInput::GetCompoundKeyStates()
	{
		std::vector<KeyState> result = {};
		for (const auto& binding : m_dirKeys)
		{
			result.push_back(binding.second.);
		}
		return result;
	}*/
	std::string CompoundInput::ToString() const
	{
		std::string actionsStringified = "";
		size_t index = 0;

		for (const auto& action : m_dirKeys)
		{
			if (index != 0) actionsStringified += ",";
			actionsStringified+=action.second.ToString();

			index++;
		}
		return std::format("[{}-> {}]", m_name, actionsStringified);
	}
}
