#include "pch.hpp"
#include "Core/Input/InputKey.hpp"
#include "Utils/Debug.hpp"

namespace Input
{
	std::string ToString(const DeviceType& device)
	{
		if (device == DeviceType::Keyboard) return "Keyboard";
		else if (device == DeviceType::Mouse) return "Mouse";
		else if (device == DeviceType::Gamepad) return "Gamepad";
		
		std::string err = std::format("Tried to convert device type to string "
			"but it has no actions sepcified");
		LogError(err);
		throw std::invalid_argument(err);
	}

	static const std::unordered_map<std::string, KeyCode>& GetKeyMap() 
	{
		static const std::unordered_map<std::string, KeyCode> map = {
		#define X(name, val) {#name, KeyCode::name},
			KEY_CODE_ENUM_LIST
		#undef X
		};
		return map;
	}

	KeyCode ToKeyCode(const std::string& str)
	{
		const auto& map = GetKeyMap();
		auto it = map.find(str);
		if (it != map.end())
			return it->second;

		return KeyCode::Null;
	}
	std::string ToString(const KeyCode keyCode)
	{
		switch (keyCode) 
		{
#define X(name, val) case KeyCode::name: return #name;
			KEY_CODE_ENUM_LIST
#undef X
			default: return "Unknown";
		}
	}

	DeviceType GetDeviceFromKeyCode(const KeyCode keyCode)
	{
		if (FIRST_MOUSE_CODE <= keyCode && keyCode <= LAST_MOUSE_CODE) return DeviceType::Mouse;
		if (FIRST_KEYBOARD_CODE <= keyCode && keyCode <= LAST_KEYBOARD_CODE) return DeviceType::Keyboard;
		if (FIRST_GAMEPAD_CODE <= keyCode && keyCode <= LAST_GAMEPAD_CODE) return DeviceType::Gamepad;
		
		LogError(std::format("Attempted to get device from key code but found no actions"));
		throw std::invalid_argument("Missing keycode to device action");
	}
	bool IsKeyCodeTextConvertible(const KeyCode keyCode)
	{
		return (keyCode >= KeyCode::Space && keyCode <= KeyCode::GraveAccent);
	}
	char GetKeyCodeAsChar(const KeyCode code, const bool isShiftPressed, const bool isCapsLockPressed)
	{
		if (!IsKeyCodeTextConvertible(code))
			return 0;

		const char textCode = static_cast<KeyCodeIntegralType>(code);
		if (code >= KeyCode::A && code <= KeyCode::Z)
		{
			//Since the keycode for alphabet chars is by default uppercase, we can directly
			//return the converted value
			if (isShiftPressed != isCapsLockPressed)
				return textCode;
			//If both cpas lock and shift is false = OR both are true (shift on caps lock makes lowercase)
			//then we do lowercase
			else return std::tolower(textCode);
		}

		if (isShiftPressed)
		{
			if (code == KeyCode::Num1)
				return '!';
			else if (code == KeyCode::Num2)
				return '@';
			else if (code == KeyCode::Num3)
				return '#';
			else if (code == KeyCode::Num4)
				return '$';
			else if (code == KeyCode::Num5)
				return '%';
			else if (code == KeyCode::Num6)
				return '^';
			else if (code == KeyCode::Num7)
				return '&';
			else if (code == KeyCode::Num8)
				return '*';
			else if (code == KeyCode::Num9)
				return '(';
			else if (code == KeyCode::Num0)
				return ')';

			else if (code == KeyCode::GraveAccent)
				return '~';
			else if (code == KeyCode::Minus)
				return '_';
			else if (code == KeyCode::Equal)
				return '+';
			else if (code == KeyCode::BracketLeft)
				return '{';
			else if (code == KeyCode::BracketRight)
				return '}';
			else if (code == KeyCode::Backslash)
				return '|';
			else if (code == KeyCode::Semicolon)
				return ':';
			else if (code == KeyCode::Apostrophe)
				return '\"';
			else if (code == KeyCode::Comma)
				return '<';
			else if (code == KeyCode::Period)
				return '>';
			else if (code == KeyCode::Slash)
				return '?';
		}

		return textCode;
	}


	std::array<KeyCode, KEYBOARD_KEY_COUNT> GetAllKeyboardKeys()
	{
		std::array<KeyCode, KEYBOARD_KEY_COUNT> keys = {};
		size_t i = 0;
		for (KeyCodeIntegralType keyCode = static_cast<KeyCodeIntegralType>(FIRST_KEYBOARD_CODE);
			keyCode <= static_cast<KeyCodeIntegralType>(LAST_KEYBOARD_CODE); keyCode++)
		{
			keys[i++] = static_cast<KeyCode>(keyCode);
		}
		return keys;
	}
	std::array<KeyCode, MOUSE_KEY_COUNT> GetAllMouseButtons()
	{
		std::array<KeyCode, MOUSE_KEY_COUNT> keys = {};
		size_t i = 0;
		for (KeyCodeIntegralType keyCode = static_cast<KeyCodeIntegralType>(FIRST_MOUSE_CODE);
			keyCode <= static_cast<KeyCodeIntegralType>(LAST_MOUSE_CODE); keyCode++)
		{
			keys[i++] = static_cast<KeyCode>(keyCode);
		}
		return keys;
	}
	std::array<KeyCode, GAMEPAD_KEY_COUNT> GetAllGamepadButtons()
	{
		std::array<KeyCode, GAMEPAD_KEY_COUNT> keys = {};
		size_t i = 0;
		for (KeyCodeIntegralType keyCode = static_cast<KeyCodeIntegralType>(FIRST_GAMEPAD_CODE);
			keyCode <= static_cast<KeyCodeIntegralType>(LAST_GAMEPAD_CODE); keyCode++)
		{
			keys[i++] = static_cast<KeyCode>(keyCode);
		}
		return keys;
	}

	InputKeyState::InputKeyState(const KeyCode& key, const InputState& state)
		: m_deviceType(GetDeviceFromKeyCode(key)), m_keyCode(key), m_state(state)
	{}

	bool InputKeyState::IsDevice(const DeviceType& device) const
	{
		return m_deviceType == device;
	}

	KeyCode InputKeyState::GetKeyCode() const
	{
		return m_keyCode;
	}

	const InputState& InputKeyState::GetState() const
	{
		return m_state;
	}
	InputState& InputKeyState::GetStateMutable()
	{
		return m_state;
	}

	std::string InputKeyState::ToString(const bool showDeviceName, const bool showState) const
	{
		std::string resultString = "";

		/*if (IsDevice(DeviceType::Keyboard)) keybindName = RaylibUtils::KeyboardKeyToString(GetAsKeyboard());
		else if (IsDevice(DeviceType::Gamepad)) keybindName = RaylibUtils::GamepadButtonToString(GetAsGamepad());
		else if (IsDevice(DeviceType::Mouse)) keybindName = RaylibUtils::MouseButtonToString(GetAsMouse());*/
		/*else
		{
			LogError(std::format("Tried to convert input key to string but device: "
				"{} failed to convert", deviceName));
			return "";
		}*/
		resultString = "[" + Input::ToString(m_keyCode);

		if (showDeviceName) resultString += std::format("({})", Input::ToString(m_deviceType));
		if (showState) resultString += std::format("->{}", m_state.ToString());
		resultString += "]";

		return resultString;
	}
}