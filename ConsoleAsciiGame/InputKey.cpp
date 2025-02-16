#include "pch.hpp"
#include "InputKey.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"

namespace Input
{
	static KeyboardKey DEFAULT_KEYBOARD_KEY = KEY_A;
	static MouseButton DEFAULT_MOUSE_BUTTON = MOUSE_BUTTON_MIDDLE;
	static GamepadButton DEFAULT_GAMEPAD_BUTTON = GAMEPAD_BUTTON_MIDDLE;

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
	std::optional<DeviceType> TryGetStringKeyDevice(const std::string& str, std::any* outInputAsEnum)
	{
		auto maybeKeyboard = RaylibUtils::TryStringToKeyboardKey(str);
		if (maybeKeyboard.has_value())
		{
			if (outInputAsEnum != nullptr) *outInputAsEnum = maybeKeyboard.value();
			return DeviceType::Keyboard;
		}

		auto maybeMouse = RaylibUtils::TryStringToMouseButton(str);
		if (maybeMouse.has_value())
		{
			if (outInputAsEnum != nullptr) *outInputAsEnum = maybeMouse.value();
			return DeviceType::Mouse;
		}

		auto maybeGamepad = RaylibUtils::TryStringToGamepadButton(str);
		if (maybeGamepad.has_value())
		{
			if (outInputAsEnum != nullptr) *outInputAsEnum = maybeGamepad.value();
			return DeviceType::Gamepad;
		}

		return std::nullopt;
	}

	InputKey::InputKey(const KeyboardKey& key, const InputState& state)
		: m_deviceType(DeviceType::Keyboard), m_keyValue(static_cast<int>(key)), m_state(state)
	{}

	InputKey::InputKey(const MouseButton& button, const InputState& state)
		: m_deviceType(DeviceType::Mouse), m_keyValue(static_cast<int>(button)), m_state(state)
	{}

	InputKey::InputKey(const GamepadButton& button, const InputState& state)
		: m_deviceType(DeviceType::Gamepad), m_keyValue(static_cast<int>(button)), m_state(state)
	{}

	bool InputKey::IsDevice(const DeviceType& device) const
	{
		return m_deviceType == device;
	}

	KeyboardKey InputKey::GetAsKeyboard() const
	{
		if (!Assert(this, IsDevice(DeviceType::Keyboard), std::format("Tried to get input key: {} as invalid device type: {}",
			std::to_string(m_keyValue), Input::ToString(m_deviceType))))
			return DEFAULT_KEYBOARD_KEY;

		return static_cast<KeyboardKey>(m_keyValue);
	}

	MouseButton InputKey::GetAsMouse() const
	{
		if (!Assert(this, IsDevice(DeviceType::Mouse), std::format("Tried to get input key: {} as invalid device type: {}",
			std::to_string(m_keyValue), Input::ToString(m_deviceType))))
			return DEFAULT_MOUSE_BUTTON;

		return static_cast<MouseButton>(m_keyValue);
	}

	GamepadButton InputKey::GetAsGamepad() const
	{
		if (!Assert(this, IsDevice(DeviceType::Gamepad), std::format("Tried to get input key: {} as invalid device type: {}",
			std::to_string(m_keyValue), Input::ToString(m_deviceType))))
			return DEFAULT_GAMEPAD_BUTTON;

		return static_cast<GamepadButton>(m_keyValue);
	}

	const InputState& InputKey::GetState() const
	{
		return m_state;
	}
	InputState& InputKey::GetStateMutable()
	{
		return m_state;
	}

	std::string InputKey::ToString() const
	{
		std::string deviceName = Input::ToString(m_deviceType);
		std::string keybindName = "";

		if (IsDevice(DeviceType::Keyboard)) keybindName = RaylibUtils::KeyboardKeyToString(GetAsKeyboard());
		else if (IsDevice(DeviceType::Gamepad)) keybindName = RaylibUtils::GamepadButtonToString(GetAsGamepad());
		else if (IsDevice(DeviceType::Mouse)) keybindName = RaylibUtils::MouseButtonToString(GetAsMouse());
		else
		{
			LogError(this, std::format("Tried to convert input key to string but device: "
				"{} failed to convert", deviceName));
			return "";
		}

		return std::format("[{}({})->{}]", keybindName, deviceName, m_state.ToString());
	}
}