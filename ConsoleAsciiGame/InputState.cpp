#include "pch.hpp"
#include "InputState.hpp"
#include "HelperFunctions.hpp"

namespace Input
{
	std::string ToString(const KeyState& state)
	{
		if (state == KeyState::Neutral) return "Neutral";
		else if (state == KeyState::Cooldown) return "Cooldown";
		else if (state == KeyState::Pressed) return "Pressed";
		else if (state == KeyState::Down) return "Down";
		else if (state == KeyState::Released) return "Up";

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
	bool InputState::IsReleased() const
	{
		return IsState(KeyState::Released);
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

	void InputState::SetCooldownTime(const float& time)
	{
		m_cooldownTime = time;
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

	std::string InputState::ToString() const
	{
		return Input::ToString(m_keyState);
	}
}