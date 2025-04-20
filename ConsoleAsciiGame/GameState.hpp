#pragma once
#include <cstdint>
#include "Event.hpp"

class GameState
{
private:
	std::uint8_t m_health;

	bool m_isPaused;

public:
	static constexpr std::uint8_t MAX_HEALTH = 5;

	/// <summary>
	/// An event that invokes whenver the health is changed via one of the 
	/// set health functions where ARG1 is the old health and ARG2 is the new health
	/// </summary>
	Event<void, std::uint8_t, std::uint8_t> m_OnHealthChanged;
	/// <summary>
	/// Invokes when the flag isPaused changes where BOOL is the new value of the flag
	/// </summary>
	Event<void, bool> m_OnGamePauseChange;

private:
public:
	GameState();

	void SetHealth(const std::uint8_t newHealth);
	void SetHealthDelta(const std::uint8_t newHealth);
	std::uint8_t GetHealth() const;

	void PauseGame();
	void UnpauseGame();
	void ToggleGamePause();
	bool IsGamePaused() const;
};

