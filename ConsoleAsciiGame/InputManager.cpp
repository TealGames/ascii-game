#include "pch.hpp"
#include "InputManager.hpp"
#include "HelperFunctions.hpp"
#include "Debug.hpp"

namespace Input
{
	const std::string InputManager::PROFILE_PREFIX = "profile_";

	InputManager::InputManager(const std::filesystem::path& allInputProfilePath) 
		: m_keyboardStates(), m_mouseStates(), m_gamepadStates(), m_profiles()
	{
		for (const auto& key : GetAllKeyboardKeys())
		{
			m_keyboardStates.emplace(key, InputKey(key, InputState()));
		}
		for (const auto& key : GetAllMouseButtons())
		{
			m_mouseStates.emplace(key, InputKey(key, InputState()));
		}
		for (const auto& key : GetAllGamepadButtons())
		{
			m_gamepadStates.emplace(key, InputKey(key, InputState()));
		}

		if (allInputProfilePath.empty()) return;

		std::string fileName = "";
		try
		{
			for (const auto& file : std::filesystem::directory_iterator(allInputProfilePath))
			{
				fileName = file.path().stem().string();
				if (!file.is_regular_file() || fileName.size() < PROFILE_PREFIX.size()) continue;
				if (fileName.substr(0, PROFILE_PREFIX.size()) != PROFILE_PREFIX) continue;

				//NOTE: we use the segment of the profile file after prefix for profile name
				CreateProfile(fileName.substr(PROFILE_PREFIX.size()), file.path());
			}
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to add all profiles at path: {} "
				"but ran into error: {}", allInputProfilePath.string(), e.what()));
		}
	}

	void InputManager::SetInputCooldown(const float& allKeyCooldownTime)
	{
		for (auto& key : m_keyboardStates)
		{
			key.second.GetStateMutable().SetCooldownTime(allKeyCooldownTime);
		}
	}
	void InputManager::SetInputCooldown(const std::map<KeyboardKey, float>& keyCooldownTime)
	{
		if (keyCooldownTime.empty()) return;

		auto it = m_keyboardStates.end();
		for (const auto& cooldownTime : keyCooldownTime)
		{
			it = m_keyboardStates.find(cooldownTime.first);
			if (it == m_keyboardStates.end()) continue;

			it->second.GetStateMutable().SetCooldownTime(cooldownTime.second);
		}
	}

	bool InputManager::IsKeyDown(const DeviceType& device, const int& keyValue)
	{
		if (device == DeviceType::Keyboard) return ::IsKeyDown(keyValue);
		else if (device == DeviceType::Mouse) return ::IsMouseButtonDown(keyValue);
		else if (device == DeviceType::Gamepad) return ::IsGamepadButtonDown(0, keyValue);
		else
		{
			LogError(std::format("Tried to check if key: '{}' of device type: {} is DOWN but this device has no actions!", 
				std::to_string(keyValue), ToString(device)));
			return false;
		}
	}

	bool InputManager::IsKeyPressed(const DeviceType& device, const int& keyValue)
	{
		if (device == DeviceType::Keyboard) return ::IsKeyPressed(keyValue);
		else if (device == DeviceType::Mouse) return ::IsMouseButtonPressed(keyValue);
		else if (device == DeviceType::Gamepad) return ::IsGamepadButtonPressed(0, keyValue);
		else
		{
			LogError(std::format("Tried to check if key: '{}' of device type: {} is PRESSED but this device has no actions!",
				std::to_string(keyValue), ToString(device)));
			return false;
		}
	}

	bool InputManager::IsKeyReleased(const DeviceType& device, const int& keyValue)
	{
		if (device == DeviceType::Keyboard) return ::IsKeyReleased(keyValue);
		else if (device == DeviceType::Mouse) return ::IsMouseButtonReleased(keyValue);
		else if (device == DeviceType::Gamepad) return ::IsGamepadButtonReleased(0, keyValue);
		else
		{
			LogError(std::format("Tried to check if key: '{}' of device type: {} is RELEASED but this device has no actions!",
				std::to_string(keyValue), ToString(device)));
			return false;
		}
	}

	void InputManager::UpdateState(const DeviceType& device, const int& keyValue, InputState& inputState, const float& deltaTime)
	{
		//First we update any deltas to cooldown
		if (inputState.InCooldown())
		{
			inputState.SetCooldownDelta(deltaTime);
			if (inputState.GetCurrentCooldownTime() >= inputState.GetCooldownTime())
			{
				inputState.ResetDefault();
			}
		}

		//We then check again (in case we might have left cooldown)
		if (!inputState.InCooldown())
		{
			if (IsKeyDown(device, keyValue))
			{
				inputState.SetState(KeyState::Down);
				//LogError(this, std::format("INput state for; {} IS: {}", std::to_string(keyValue), ToString(inputState.GetState())));
			}
			else if (IsKeyPressed(device, keyValue)) inputState.SetState(KeyState::Pressed);
			else if (IsKeyReleased(device, keyValue)) inputState.SetState(KeyState::Released);
			else
			{
				//If we are not pressing anything, but last frame we released and we have cooldown
				//we only set the state for cooldown so it gets updated next frame
				if (inputState.IsReleased() && inputState.HasCooldown())
				{
					inputState.SetState(KeyState::Cooldown);
				}
				//Othwewise no cooldown means we can just go back to neutral
				else inputState.SetState(KeyState::Neutral);
			}
		}
	}

	void InputManager::Update(const float& deltaTime)
	{
		for (auto& inputState : m_keyboardStates)
		{
			UpdateState(DeviceType::Keyboard, static_cast<int>(inputState.first), 
				inputState.second.GetStateMutable(), deltaTime);
		}

		for (auto& inputState : m_mouseStates)
		{
			UpdateState(DeviceType::Mouse, static_cast<int>(inputState.first), 
				inputState.second.GetStateMutable(), deltaTime);
		}

		for (auto& inputState : m_gamepadStates)
		{
			UpdateState(DeviceType::Gamepad, static_cast<int>(inputState.first), 
				inputState.second.GetStateMutable(), deltaTime);
		}
	}

	void InputManager::CreateProfile(const std::string& name, const std::filesystem::path& profilePath)
	{
		m_profiles.emplace(name, InputProfile(*this, name, profilePath));
	}
	const InputProfile* InputManager::TryGetProfile(const std::string& name) const
	{
		auto it = m_profiles.find(name);
		if (it == m_profiles.end()) return nullptr;

		return &(it->second);
	}

	std::vector<KeyboardKey> InputManager::GetAllKeyboardKeys()
	{
		std::vector<KeyboardKey> keys = {};
		for (int keyCode = static_cast<int>(KEY_BACK); 
			keyCode <= static_cast<int>(KEY_KB_MENU); keyCode++)
		{
			keys.push_back(static_cast<KeyboardKey>(keyCode));
		}
		return keys;
	}
	std::vector<MouseButton> InputManager::GetAllMouseButtons()
	{
		std::vector<MouseButton> keys = {};
		for (int keyCode = static_cast<int>(MOUSE_BUTTON_LEFT); 
			keyCode <= static_cast<int>(MOUSE_BUTTON_BACK); keyCode++)
		{
			keys.push_back(static_cast<MouseButton>(keyCode));
		}
		return keys;
	}
	std::vector<GamepadButton> InputManager::GetAllGamepadButtons()
	{
		std::vector<GamepadButton> keys = {};
		for (int keyCode = static_cast<int>(GAMEPAD_BUTTON_LEFT_FACE_UP); 
			keyCode <= static_cast<int>(GAMEPAD_BUTTON_RIGHT_THUMB); keyCode++)
		{
			keys.push_back(static_cast<GamepadButton>(keyCode));
		}
		return keys;
	}

	/*CompoundInputCollection::iterator TryGetCompoundIteratorMutable(const std::string& name)
	{
		return CompoundInput.find(name);
	}
	CompoundInputCollection::const_iterator TryGetCompoundIterator(const std::string& name)
	{
		return CompoundInput.find(name);
	}*/

	bool InputManager::IsKeyState(const KeyboardKey& key, const KeyState& state)
	{
		auto it = m_keyboardStates.find(key);
		if (it == m_keyboardStates.end()) return false;

		return it->second.GetState().IsState(state);
	}
	const KeyState& InputManager::GetKeyState(const KeyboardKey& key)
	{
		auto it = m_keyboardStates.find(key);
		if (!Assert(it != m_keyboardStates.end(), std::format("InputManager: Tried to get key state in for key: {} "
			"but that key does not exist", std::to_string(key))))
			throw std::invalid_argument("Invalid key state");

		return it->second.GetState().GetState();
	}
	bool InputManager::IsKeyPressed(const KeyboardKey& key)
	{
		return IsKeyState(key, KeyState::Pressed);
	}
	bool InputManager::IsKeyDown(const KeyboardKey& key)
	{
		return IsKeyState(key, KeyState::Down);
	}
	bool InputManager::IsKeyReleased(const KeyboardKey& key)
	{
		return IsKeyState(key, KeyState::Released);
	}

	const InputKey* InputManager::GetInputKey(const KeyboardKey& key)
	{
		auto it = m_keyboardStates.find(key);
		if (it != m_keyboardStates.end()) return &(it->second);
		
		return nullptr;
	}
	const InputKey* InputManager::GetInputKey(const MouseButton& button)
	{
		auto it = m_mouseStates.find(button);
		if (it != m_mouseStates.end()) return &(it->second);

		return nullptr;
	}
	const InputKey* InputManager::GetInputKey(const GamepadButton& button)
	{
		auto it = m_gamepadStates.find(button);
		if (it != m_gamepadStates.end()) return &(it->second);

		return nullptr;
	}

	/*bool TryAddCompoundInput(const std::string& name, const CompoundDirectionCollection& keys)
	{
		if (!Assert(TryGetCompoundIterator(name) == CompoundInput.end(),
			std::format("InputManager: Tried to add compound input with name: '{}' "
				"but that compound input binding already exists", name)))
			return false;

		CompoundInput.emplace(name, keys);
		return true;
	}*/
}