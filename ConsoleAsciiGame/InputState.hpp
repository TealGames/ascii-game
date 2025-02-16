#pragma once
#include <string>

namespace Input
{
	enum class KeyState
	{
		Neutral,
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
		float m_currentCooldownTime;

	public:
		/// <summary>
		/// This is used on the cooldowmn time var to signify no cooldown
		/// </summary>
		static constexpr float NO_COOLDOWN_TIME = -1;

	private:
		InputState(const KeyState& startState, const float& cooldownTime);
	public:
		InputState();
		InputState(const float& cooldown);

		bool HasCooldown() const;
		bool InCooldown() const;
		void ResetDefault();
		const float& GetCooldownTime() const;
		const float& GetCurrentCooldownTime() const;

		void SetCooldownTime(const float& time);
		void SetCooldownDelta(const float& delta);

		const KeyState& GetState() const;
		void SetState(const KeyState& newState);
		bool IsState(const KeyState& state) const;

		bool IsPressed() const;
		bool IsDown() const;
		bool IsReleased() const;

		std::string ToString() const;
	};
}

