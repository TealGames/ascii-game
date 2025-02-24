#pragma once
#include "raylib.h"
#include "Debug.hpp"
#include <cstdint>
#include <string>
#include <optional>
#include <any>
#include "InputState.hpp"

namespace Input
{
	enum class DeviceType
	{
		Keyboard,
		Mouse,
		Gamepad,
	};
	std::string ToString(const DeviceType& device);
	std::optional<DeviceType> TryGetStringKeyDevice(const std::string& str, std::any* outInputAsEnum=nullptr);

	class InputKey
	{
	private:
		DeviceType m_deviceType;
		int m_keyValue;
		InputState m_state;

	public:

	private:
	public:
		InputKey(const KeyboardKey& key, const InputState& state);
		InputKey(const MouseButton& button, const InputState& state);
		InputKey(const GamepadButton & button, const InputState& state);

		bool IsDevice(const DeviceType& device) const;

		KeyboardKey GetAsKeyboard() const;
		MouseButton GetAsMouse() const;
		GamepadButton GetAsGamepad() const;

		const InputState& GetState() const;
		InputState& GetStateMutable();

		std::string ToString(const bool showDeviceName=true, const bool showState=true) const;
	};
}
