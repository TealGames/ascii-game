#include "pch.hpp"
#include "InputManager.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

std::string ToString(const KeyState& state)
{
	if (state == KeyState::Neutral) return "Neutral";
	else if (state == KeyState::Cooldown) return "Cooldown";
	else if (state == KeyState::Pressed) return "Pressed";
	else if (state == KeyState::Down) return "Down";
	else if (state == KeyState::Up) return "Up";

	std::string err = std::format("Failed to get string of key state");
	throw std::invalid_argument(err);
	return "";
}

InputState::InputState() : InputState(KeyState::Neutral, InputState::NO_COOLDOWN_TIME) {}
InputState::InputState(const float& cooldown) : 
	InputState(KeyState::Neutral, cooldown) {}

InputState::InputState(const KeyState& startState, const float& cooldownTime) :
	m_keyState(startState), m_cooldownTime(cooldownTime), m_currentCooldownTime()
{
}

const KeyState& InputState::GetState() const
{
	return m_keyState;
}
void InputState::SetState(const KeyState& newState)
{
	m_keyState = newState;
}
bool InputState::IsState(const KeyState& state) const
{
	return m_keyState == state;
}
bool InputState::IsPressed() const
{
	return IsState(KeyState::Pressed);
}
bool InputState::IsDown() const
{
	return IsState(KeyState::Down);
}
bool InputState::IsUp() const
{
	return IsState(KeyState::Up);
}
bool InputState::InCooldown() const
{
	return IsState(KeyState::Cooldown);
}
const float& InputState::GetCooldownTime() const
{
	return m_cooldownTime;
}
const float& InputState::GetCurrentCooldownTime() const
{
	return m_currentCooldownTime;
}
bool InputState::HasCooldown() const
{
	return !Utils::ApproximateEqualsF(m_cooldownTime, InputState::NO_COOLDOWN_TIME);
}

void InputState::SetCooldownDelta(const float& delta)
{
	if (!HasCooldown()) return;

	m_currentCooldownTime += delta;
}
void InputState::ResetDefault()
{
	m_currentCooldownTime = 0;
	SetState(KeyState::Neutral);
}

CompoundInput::CompoundInput(const CompoundDirectionCollection& keys) :
	m_dirKeys(keys) {}

std::optional<KeyboardKey> CompoundInput::TryGetDirection(const Direction& dir) const
{
	auto it = m_dirKeys.find(dir);
	if (it != m_dirKeys.end()) return it->second;

	return std::nullopt;
}
const CompoundDirectionCollection& CompoundInput::GetEntries() const
{
	return m_dirKeys;
}
std::size_t CompoundInput::GetEntriesCount() const
{
	return m_dirKeys.size();
}

InputManager::InputManager() : InputManager(std::map<KeyboardKey, float>{}) {}
InputManager::InputManager(const float& allKeyCooldownTime) : m_keyStates()
{
	for (const auto& key : GetAllKeys())
	{
		m_keyStates.emplace(key, InputState(allKeyCooldownTime));
	}
}
InputManager::InputManager(const std::map<KeyboardKey, float>& keyCooldownTime) : m_keyStates()
{
	auto it = keyCooldownTime.begin();
	for (const auto& key : GetAllKeys())
	{
		if (!keyCooldownTime.empty() && it!= keyCooldownTime.end() && key == it->first)
		{
			m_keyStates.emplace(key, InputState(it->second));
			it++;
		}
		else m_keyStates.emplace(key, InputState());
	}
}

std::vector<KeyboardKey> InputManager::GetAllKeys() const
{
	std::vector<KeyboardKey> keys = {};
	for (int keyCode = static_cast<int>(KEY_BACK); keyCode <= static_cast<int>(KEY_KB_MENU); keyCode++)
	{
		keys.push_back(static_cast<KeyboardKey>(keyCode));
	}
	return keys;
}

KeyStateCollection::iterator InputManager::TryGetKeyIteratorMutable(const KeyboardKey& key)
{
	return m_keyStates.find(key);
}
KeyStateCollection::const_iterator InputManager::TryGetKeyIterator(const KeyboardKey& key) const
{
	return m_keyStates.find(key);
}

CompoundInputCollection::iterator InputManager::TryGetCompoundIteratorMutable(const std::string& name)
{
	return m_compoundInput.find(name);
}
CompoundInputCollection::const_iterator InputManager::TryGetCompoundIterator(const std::string& name) const
{
	return m_compoundInput.find(name);
}

bool InputManager::IsKeyState(const KeyboardKey& key, const KeyState& state) const
{
	auto it = TryGetKeyIterator(key);
	if (it == m_keyStates.end()) return false;

	return it->second.IsState(state);
}
const KeyState& InputManager::GetKeyState(const KeyboardKey& key) const
{
	auto it = TryGetKeyIterator(key);
	if (!Assert(this, it != m_keyStates.end(), std::format("Tried to get key state for key: {} "
		"but that key does not exist", std::to_string(key))))
		throw std::invalid_argument("Invalid key state");

	return it->second.GetState();
}
bool InputManager::IsKeyPressed(const KeyboardKey& key) const
{
	return IsKeyState(key, KeyState::Pressed);
}
bool InputManager::IsKeyDown(const KeyboardKey& key) const
{
	return IsKeyState(key, KeyState::Down);
}
bool InputManager::IsKeyUp(const KeyboardKey& key) const
{
	return IsKeyState(key, KeyState::Up);
}

bool InputManager::TryAddCompoundInput(const std::string& name, const CompoundDirectionCollection& keys)
{
	if (!Assert(this, TryGetCompoundIterator(name) == m_compoundInput.end(),
		std::format("Tried to add compound input with name: '{}' "
			"but that compound input binding already exists", name)))
		return false;

	m_compoundInput.emplace(name, keys);
	return true;
}
std::optional<KeyboardKey> InputManager::TryGetCompoundDirection(const std::string& name, const Direction& dir) const
{
	auto it = TryGetCompoundIterator(name);
	if (!Assert(this, TryGetCompoundIterator(name) != m_compoundInput.end(),
		std::format("Tried to get direction of compound input with name: '{}' "
			"but that compound input binding does not exist", name)))
		return std::nullopt;

	return it->second.TryGetDirection(dir);
}
std::optional<Utils::Point2DInt> InputManager::TryGetCompoundInputDown(const std::string& name) const
{
	auto it = TryGetCompoundIterator(name);
	if (!Assert(this, TryGetCompoundIterator(name) != m_compoundInput.end(),
		std::format("Tried to get compound input vector for down with name: '{}' "
			"but that compound input binding does not exist", name)))
		return std::nullopt;

	Utils::Point2DInt dir = {};
	for (const auto& entry : it->second.GetEntries())
	{
		LogError(this, std::format("Compound: {} key: {} STATE: {} (down: {})", name, std::to_string(entry.second), 
			ToString(GetKeyState(entry.second)), std::to_string(IsKeyDown(entry.second))));
		if (!IsKeyDown(entry.second)) continue;

		if (entry.first == Direction::Up) dir.m_Y++;
		else if (entry.first == Direction::Down) dir.m_Y--;
		else if (entry.first == Direction::Right) dir.m_X++;
		else if (entry.first == Direction::Left) dir.m_X--;
	}
	LogError(this, std::format("When retrieving compound: {} -> {}", name, dir.ToString()));
	return dir;
}
std::vector<KeyState> InputManager::GetCompoundKeyStates(const std::string& name) const
{
	auto it = TryGetCompoundIterator(name);
	if (!Assert(this, TryGetCompoundIterator(name) != m_compoundInput.end(),
		std::format("Tried to get compound to string for compound: '{}' "
			"but that compound input binding does not exist", name)))
		return {};

	std::vector<KeyState> result = {};
	for (const auto& binding : it->second.GetEntries())
	{
		result.push_back(GetKeyState(binding.second));
	}
	return result;
}
std::string InputManager::GetCompoundToString(const std::string& name) const
{
	return Utils::ToStringIterable<std::vector<KeyState>, KeyState>(GetCompoundKeyStates(name));
}

void InputManager::Update(const float& deltaTime)
{
	for (auto& inputState : m_keyStates)
	{
		//First we update any deltas to cooldown
		if (inputState.second.InCooldown())
		{
			inputState.second.SetCooldownDelta(deltaTime);
			if (inputState.second.GetCurrentCooldownTime() >= inputState.second.GetCooldownTime())
			{
				inputState.second.ResetDefault();
			}
		}

		//We then check again (in case we might have left cooldown)
		if (!inputState.second.InCooldown())
		{
			if (::IsKeyDown(inputState.first))
			{
				inputState.second.SetState(KeyState::Down);
				//LogError(this, std::format("INput state for; {} IS: {}", std::to_string(inputState.first), ToString(inputState.second.GetState())));
			}
			else if (::IsKeyPressed(inputState.first)) inputState.second.SetState(KeyState::Pressed);
			else if (::IsKeyReleased(inputState.first)) inputState.second.SetState(KeyState::Up);
			else
			{
				//If we are not pressing anything, but last frame we released and we have cooldown
				//we only set the state for cooldown so it gets updated next frame
				if (inputState.second.IsUp() && inputState.second.HasCooldown())
				{
					inputState.second.SetState(KeyState::Cooldown);
				}
				//Othwewise no cooldown means we can just go back to neutral
				else inputState.second.SetState(KeyState::Neutral);
			}
		}
	}
}