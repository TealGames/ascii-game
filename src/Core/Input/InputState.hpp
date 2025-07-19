#pragma once
#include <string>
#include <cstdint>

namespace Input
{
	enum class KeyState : std::uint8_t
	{
		Neutral= 0,
		Cooldown,
		Pressed,
		Down,
		Released,
	};
	std::string ToString(const KeyState& state);

	class InputState
	{
	private:
		KeyState m_keyState;
		float m_cooldownTime;
		/// <summary>
		/// The current amount of time passed for the total cooldown
		/// </summary>
		float m_currentCooldownTime;
		/// <summary>
		/// The amount of time that the key is pressed down for
		/// </summary>
		float m_downTime;

	public:
		/// <summary>
		/// This is used on the cooldowmn time var to signify no cooldown
		/// </summary>
		static constexpr float NO_COOLDOWN_TIME = -1;

	private:
		InputState(const KeyState startState, const float& cooldownTime);
		void ResetDefault();

	public:
		InputState();
		InputState(const float& cooldown);

		bool HasCooldown() const;
		bool InCooldown() const;

		float GetCooldownTime() const;
		float GetCurrentCooldownTime() const;
		void SetCooldownTime(const float& time);
		void SetCooldownDelta(const float& delta);

		float GetCurrentDownTime() const;
		void SetDownTimeDelta(float deltaTime);

		const KeyState GetState() const;
		void SetState(const KeyState newState);
		bool IsState(const KeyState state) const;

		bool IsPressed() const;
		bool IsDown() const;
		bool IsDownForTime(const float downTime) const;
		bool IsReleased() const;

		std::string ToString() const;
	};
}

